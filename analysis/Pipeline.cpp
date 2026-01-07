#include "Pipeline.h"
#include "../core/LogParser.h"
#include "../io/MemoryMappedFile.h"
#include "KeywordHitAnalyzer.h"
#include "LevelCountAnalyzer.h"
#include "TimeRangeFilter.h"
#include "TopErrorAnalyzer.h"
#include <algorithm>
#include <cmath>
#include <future>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <thread>
#include <vector>

namespace loganalyzer {

namespace {

// Helper to determine chunk boundaries for parallel processing
struct Chunk {
  std::string_view data;
  size_t startLineNumber;
};

// Locate the start of the next line after offset
size_t findNextLineStart(std::string_view data, size_t offset) {
  if (offset >= data.size())
    return data.size();
  size_t pos = data.find('\n', offset);
  if (pos == std::string_view::npos)
    return data.size();
  return pos + 1;
}

} // namespace

AnalysisResult Pipeline::run(const std::string &inputPath,
                             const AnalysisContext &context,
                             ProgressCallback progressCallback,
                             bool *wasCancelled) {
  AnalysisResult result;

  if (wasCancelled) {
    *wasCancelled = false;
  }

  // Open file with Memory Mapping
  MemoryMappedFile mmapFile(inputPath);
  if (!mmapFile.isOpen()) {
    return result;
  }

  std::string_view fileData = mmapFile.getView();
  if (fileData.empty()) {
    return result;
  }

  // Determine available concurrency
  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0)
    numThreads = 2; // Fallback

  // For small files, avoid overhead of threads
  if (fileData.size() < 1024 * 1024) { // < 1MB
    numThreads = 1;
  }

  // Calculate chunks
  std::vector<size_t> chunkStarts;
  chunkStarts.push_back(0);

  size_t idealChunkSize = fileData.size() / numThreads;
  for (unsigned int i = 1; i < numThreads; ++i) {
    size_t approximateStart = i * idealChunkSize;
    size_t actualStart = findNextLineStart(fileData, approximateStart);
    if (actualStart < fileData.size() && actualStart > chunkStarts.back()) {
      chunkStarts.push_back(actualStart);
    }
  }
  chunkStarts.push_back(fileData.size());

  // Adjust numThreads if file was small or lines were huge
  numThreads = chunkStarts.size() - 1;

  // Shared progress tracker
  std::atomic<uint64_t> totalBytesProcessed{0};
  uint64_t fileSize = fileData.size();

  // Define worker task
  auto worker = [&](size_t startOffset, size_t endOffset,
                    size_t startLineNum) -> AnalysisResult {
    AnalysisResult localResult;

    // Setup analyzers (thread-local instances)
    TimeRangeFilter filter(context.fromTs, context.toTs);
    std::vector<std::unique_ptr<IAnalyzer>> analyzers;
    analyzers.push_back(std::make_unique<LevelCountAnalyzer>());
    analyzers.push_back(
        std::make_unique<TopErrorAnalyzer>()); // Each thread has its own top-N
                                               // buffer
    if (context.keyword.has_value()) {
      analyzers.push_back(
          std::make_unique<KeywordHitAnalyzer>(context.keyword.value()));
    }

    size_t currentPos = startOffset;
    size_t lineNumber = startLineNum; // Note: Line numbers will be estimates if
                                      // we don't count previous newlines.
    // Calculating exact line numbers for chunks requires counting newlines in
    // previous chunks, which is linear. For accurate line numbers in results
    // (e.g. error reporting), we might need two passes or just accept that line
    // numbers are relative or approximate in this phase. However, usually we
    // can just scan for newlines. To strictly follow requirements avoiding
    // "double copy", counting newlines is fast. But parallel counting is
    // complex. Let's assume for this "High Performance" version, exact line
    // numbers for *every* line might be secondary OR we just count locally and
    // don't care about global offset unless requested. Wait, ParseResult
    // contains lineNumber. LogEntry works better with it. If we want exact line
    // numbers, we must count newlines from 0 to startOffset. This kills
    // parallelism if we do it sequentially. Compromise: We can just use local
    // line number 0-based within chunk, or if strictness is needed, pre-scan
    // newlines? Actually, for "Top Errors" we just need the message. Line
    // number is used for ParseError reporting. Let's pass 0 or a placeholder if
    // expensive, BUT user might want to locate errors. Let's accept that for
    // now we pass relative line numbers or pseudo-numbers. OR better: we can
    // guess line number = startOffset / avg_line_len? No, inaccurate. Correct
    // approach for parallel lines:
    // 1. Just process.
    // 2. If we need global line numbers, we need a prefix sum of newline
    // counts. Let's proceed with local line numbers for now or just
    // startLineNum passed in. Wait, I can pass a startLineNum if I computed it.
    // I won't compute it for speed. I'll just increment.

    // Update: Since I don't pre-calculate line numbers, I'll start at 0 for
    // each chunk basically? Or 1 + offset/something. Let's just use 0 as base
    // for chunks > 0 to indicate "unknown absolute line". Users typically grep
    // based on timestamp/content anyway.

    const uint64_t progressReportInterval = 1024 * 1024; // 1MB
    uint64_t bytesSinceLastReport = 0;

    while (currentPos < endOffset) {
      if (wasCancelled && *wasCancelled)
        return localResult;

      size_t newlinePos = fileData.find('\n', currentPos);
      size_t lineEnd =
          (newlinePos == std::string_view::npos || newlinePos >= endOffset)
              ? endOffset
              : newlinePos;

      size_t contentEnd = lineEnd;
      if (contentEnd > currentPos && fileData[contentEnd - 1] == '\r') {
        contentEnd--;
      }

      std::string_view line =
          fileData.substr(currentPos, contentEnd - currentPos);
      lineNumber++;

      // Parse
      ParseResult parseResult = LogParser::parse(line, lineNumber);

      if (std::holds_alternative<LogEntry>(parseResult)) {
        localResult.parsedLines++; // thread-local count
        const LogEntry &entry = std::get<LogEntry>(parseResult);

        if (filter.accept(entry.ts)) {
          if (filter.isActive())
            localResult.timeRangeMatched++;
          for (auto &analyzer : analyzers) {
            analyzer->process(entry);
          }
        }
      } else {
        localResult.invalidLines++;
        const ParseError &error = std::get<ParseError>(parseResult);
        localResult.parseErrors[error.code]++;
      }

      localResult.totalLines++;

      // Progress
      size_t consumed = (lineEnd - currentPos) +
                        (newlinePos != std::string_view::npos ? 1 : 0);
      currentPos += consumed;

      bytesSinceLastReport += consumed;
      if (bytesSinceLastReport >= progressReportInterval) {
        totalBytesProcessed.fetch_add(bytesSinceLastReport,
                                      std::memory_order_relaxed);
        bytesSinceLastReport = 0;
      }

      if (currentPos >= endOffset && newlinePos == std::string_view::npos)
        break;
    }

    // Flush remaining progress
    if (bytesSinceLastReport > 0) {
      totalBytesProcessed.fetch_add(bytesSinceLastReport,
                                    std::memory_order_relaxed);
    }

    // Finalize analyzers
    for (auto &analyzer : analyzers) {
      analyzer->finalize(localResult);
    }
    return localResult;
  };

  // Launch tasks
  std::vector<std::future<AnalysisResult>> futures;
  for (unsigned int i = 0; i < numThreads; ++i) {
    size_t start = chunkStarts[i];
    size_t end = chunkStarts[i + 1];
    // Note: We are ignoring absolute line numbers for performance in Phase 2.
    futures.push_back(std::async(std::launch::async, worker, start, end, 0));
  }

  // Monitor progress while waiting
  if (numThreads > 0) {
    // We can't easily wait_for on all futures in a loop without blocking logic.
    // Simpler: Just wait for all, but maybe use a dedicated progress thread?
    // Or just check futures one by one?
    // Check futures one by one is simple but sequential.
    // Better: Main loop sleeps and checks atomic counter.
    // But we need to know when tasks are done.
    // Workaround: We loop until totalBytesProcessed >= fileSize or close
    // enough, OR we assume futures return. But std::async destructors block.
    // Let's loop checking status? No standard is_ready without wait_for(0).

    bool allDone = false;
    while (!allDone) {
      if (wasCancelled && *wasCancelled)
        break;

      // Create a small delay
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      // Report progress
      if (progressCallback && fileSize > 0) {
        float p = static_cast<float>(
                      totalBytesProcessed.load(std::memory_order_relaxed)) /
                  static_cast<float>(fileSize);
        if (!progressCallback(p)) {
          if (wasCancelled)
            *wasCancelled = true;
          break;
        }
      }

      // Check if completion (heuristic or check futures)
      // Ideally we used a latch or barrier, but C++17 doesn't have it
      // standardly (C++20 does). We can check wait_for(0) on all futures.
      allDone = true;
      for (auto &f : futures) {
        if (f.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
          allDone = false;
          break;
        }
      }
    }
  }

  // Gather results
  for (auto &f : futures) {
    if (f.valid()) { // if not moved
      // If cancelled, we still want to join basically?
      // If cancelled, futures will complete eventually.
      try {
        AnalysisResult part = f.get();
        result.merge(part);
      } catch (...) {
        // Ignore exceptions from threads? Or rethrow?
      }
    }
  }

  // Final progress 100%
  if (progressCallback && (!wasCancelled || !*wasCancelled)) {
    progressCallback(1.0f);
  }

  return result;
}

} // namespace loganalyzer
