cmake_minimum_required(VERSION 3.10)
project(CompilerGUI)

# Find required packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    main.cpp
    MainWindow.cpp
)

# Add header files
set(HEADERS
    MainWindow.hpp
)

# Create the executable
add_executable(CompilerGUI WIN32 ${SOURCES} ${HEADERS})

# Link against Qt libraries and lexer
target_link_libraries(CompilerGUI PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    lexer
)

# Include directories
target_include_directories(CompilerGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lexer
)

# Enable automatic MOC and UIC
set_target_properties(CompilerGUI PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    OUTPUT_NAME "CompilerGUI"
)

# Copy Qt DLLs to output directory on Windows
if(WIN32)
    add_custom_command(TARGET CompilerGUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE:Qt6::Widgets>
            $<TARGET_FILE_DIR:CompilerGUI>
    )
endif() 