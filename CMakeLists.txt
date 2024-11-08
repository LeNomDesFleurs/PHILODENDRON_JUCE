# to download JUCE
# git submodule update --init --recursive 

cmake_minimum_required(VERSION 3.15)

PROJECT(JUCE_EXAMPLE
    LANGUAGES CXX
    VERSION 0.0.2
)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)

set(IS_JUCE_INSTALLED OFF)



# for clang-tidy(this enable to find system header files).
if(CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Enable JUCE. Do not use find_package to prevent from mix up with one globally installed.
add_subdirectory(lib/JUCE)

add_subdirectory(src)