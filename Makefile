# Compiler settings
CXX = clang++
CXXFLAGS = -std=c++17 -stdlib=libc++ -O2 -Wall -Wextra -Wpedantic

# Directories
CORE_DIR = core
ANALYSIS_DIR = analysis
IO_DIR = io
APP_DIR = app
REPORT_DIR = report
TESTS_DIR = tests

# Source files for CLI
CORE_SRCS = $(CORE_DIR)/Timestamp.cpp $(CORE_DIR)/LogParser.cpp
ANALYSIS_SRCS = $(ANALYSIS_DIR)/LevelCountAnalyzer.cpp \
                $(ANALYSIS_DIR)/KeywordHitAnalyzer.cpp \
                $(ANALYSIS_DIR)/TopErrorAnalyzer.cpp \
                $(ANALYSIS_DIR)/TimeRangeFilter.cpp \
                $(ANALYSIS_DIR)/Pipeline.cpp
IO_SRCS = $(IO_DIR)/FileReader.cpp $(IO_DIR)/FileWriter.cpp
APP_SRCS = $(APP_DIR)/Application.cpp
REPORT_SRCS = $(REPORT_DIR)/TextReportRenderer.cpp

ALL_SRCS = $(CORE_SRCS) $(ANALYSIS_SRCS) $(IO_SRCS) $(APP_SRCS) $(REPORT_SRCS)

# Object files
CORE_OBJS = $(CORE_SRCS:.cpp=.o)
ANALYSIS_OBJS = $(ANALYSIS_SRCS:.cpp=.o)
IO_OBJS = $(IO_SRCS:.cpp=.o)
APP_OBJS = $(APP_SRCS:.cpp=.o)
REPORT_OBJS = $(REPORT_SRCS:.cpp=.o)

ALL_OBJS = $(CORE_OBJS) $(ANALYSIS_OBJS) $(IO_OBJS) $(APP_OBJS) $(REPORT_OBJS)

# Executables
CLI_TARGET = log_analyzer
TEST_PARSER = test_parser
TEST_ANALYZERS = test_analyzers

# Default target
.PHONY: all
all: build-cli

# Build CLI
.PHONY: build-cli
build-cli: $(CLI_TARGET)

$(CLI_TARGET): $(ALL_OBJS) main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build tests
.PHONY: build-tests
build-tests: $(TEST_PARSER) $(TEST_ANALYZERS)

$(TEST_PARSER): $(CORE_OBJS) $(TESTS_DIR)/test_parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_ANALYZERS): $(CORE_OBJS) $(ANALYSIS_OBJS) $(IO_OBJS) $(TESTS_DIR)/test_analyzers.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TESTS_DIR)/test_parser.o: $(TESTS_DIR)/test_parser.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TESTS_DIR)/test_analyzers.o: $(TESTS_DIR)/test_analyzers.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# GUI-specific compilation rules (must come before generic %.o rule)
external/imgui/%.o: external/imgui/%.cpp
	$(CXX) $(GUI_CXXFLAGS) -c -o $@ $<

external/imgui/backends/%.o: external/imgui/backends/%.cpp
	$(CXX) $(GUI_CXXFLAGS) -c -o $@ $<

gui/%.o: gui/%.cpp
	$(CXX) $(GUI_CXXFLAGS) -c -o $@ $<

# Generic rule for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Run tests
.PHONY: test
test: build-tests
	@echo "Running parser tests..."
	@./$(TEST_PARSER)
	@echo ""
	@echo "Running analyzer tests..."
	@./$(TEST_ANALYZERS)
	@echo ""
	@echo "All tests completed!"

# Run with sample log
.PHONY: run
run: build-cli
	./$(CLI_TARGET) --input tests/sample_log.txt --report out_report.txt
	@echo "Report generated: out_report.txt"
	@cat out_report.txt

# Clean
.PHONY: clean
clean:
	rm -f $(ALL_OBJS) main.o $(TESTS_DIR)/*.o
	rm -f $(GUI_OBJS) $(IMGUI_OBJS) $(GUI_DIR)/main_gui.o
	rm -f $(CLI_TARGET) $(TEST_PARSER) $(TEST_ANALYZERS) $(GUI_TARGET)
	rm -f out_report.txt

# GUI dependencies
GUI_DIR = gui
IMGUI_DIR = external/imgui
IMGUI_SRCS = $(IMGUI_DIR)/imgui.cpp \
             $(IMGUI_DIR)/imgui_draw.cpp \
             $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
GUI_SRCS = $(GUI_DIR)/GuiController.cpp
IMGUI_OBJS = $(IMGUI_SRCS:.cpp=.o)
GUI_OBJS = $(GUI_SRCS:.cpp=.o)

GUI_TARGET = log_analyzer_gui
GUI_CXXFLAGS = $(CXXFLAGS) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
GUI_LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
GUI_LIBS += -L/usr/local/lib -lglfw

# GUI target
.PHONY: build-gui
build-gui: $(GUI_TARGET)

$(GUI_TARGET): $(CORE_OBJS) $(ANALYSIS_OBJS) $(IO_OBJS) $(APP_OBJS) $(IMGUI_OBJS) $(GUI_OBJS) $(GUI_DIR)/main_gui.o
	$(CXX) $(GUI_CXXFLAGS) -o $@ $^ $(GUI_LIBS)

$(GUI_DIR)/main_gui.o: $(GUI_DIR)/main_gui.cpp
	$(CXX) $(GUI_CXXFLAGS) -c -o $@ $<
