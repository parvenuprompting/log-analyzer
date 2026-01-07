cmake_minimum_required(VERSION 3.14)
project(LogAnalyzer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_library(COCOA_LIB Cocoa)
find_library(IOKIT_LIB IOKit)
find_library(COREVIDEO_LIB CoreVideo)

# --- ImGui Library ---
set(IMGUI_DIR "external/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
)
# Link GLFW so that ImGui can find <GLFW/glfw3.h>
target_link_libraries(imgui PUBLIC glfw)

# --- Core Sources ---
set(CORE_SOURCES
    core/StandardLogParser.cpp
    core/PatternLogParser.cpp
    core/Timestamp.cpp
    core/ConfigManager.cpp
    io/MemoryMappedFile.cpp
    io/FileWriter.cpp
    analysis/LevelCountAnalyzer.cpp
    analysis/KeywordHitAnalyzer.cpp
    analysis/TopErrorAnalyzer.cpp
    analysis/TimeRangeFilter.cpp
    analysis/AnalysisResult.cpp
    analysis/Pipeline.cpp
    app/Application.cpp
)

# --- CLI Executable ---
add_executable(log_analyzer
    ${CORE_SOURCES}
    report/TextReportRenderer.cpp
    main.cpp
)

# --- GUI Executable ---
add_executable(log_analyzer_gui MACOSX_BUNDLE
    ${CORE_SOURCES}
    gui/GuiController.cpp
    gui/LogViewer.cpp
    gui/main_gui.cpp
    resources/AppIcon.icns
    resources/background_calm.png
    resources/fa-solid-900.ttf
)

set_target_properties(log_analyzer_gui PROPERTIES
    MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
    RESOURCE "resources/AppIcon.icns;resources/background_calm.png;resources/fa-solid-900.ttf"
)
# Link imgui (which links glfw), and macOS frameworks
target_link_libraries(log_analyzer_gui PRIVATE imgui ${COCOA_LIB} ${IOKIT_LIB} ${COREVIDEO_LIB} OpenGL::GL)

# --- Tests ---
enable_testing()

add_executable(unit_tests
    ${CORE_SOURCES}
    tests/test_parser_catch2.cpp
    tests/test_analyzers_catch2.cpp
    tests/test_pattern_parser.cpp
    tests/test_main_catch2.cpp
    external/catch2/catch_amalgamated.cpp
)
target_include_directories(unit_tests PRIVATE external/catch2)

add_test(NAME AllTests COMMAND unit_tests)
