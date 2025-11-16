cmake_minimum_required(VERSION 3.10)
project(GolombCoding)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Golomb coding test executable
add_executable(golomb_test
    main.cpp
    GolombCoding.cpp
    GolombCoding.h
)

# Audio codec test executable
add_executable(audio_test
    audio_test.cpp
    AudioCodec.cpp
    AudioCodec.h
    WAVFile.cpp
    WAVFile.h
    GolombCoding.cpp
    GolombCoding.h
)

# Optional: Add compiler warnings
if(MSVC)
    target_compile_options(golomb_test PRIVATE /W4)
    target_compile_options(audio_test PRIVATE /W4)
else()
    target_compile_options(golomb_test PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(audio_test PRIVATE -Wall -Wextra -pedantic)
endif()

# Add M_PI definition for MSVC
if(MSVC)
    target_compile_definitions(audio_test PRIVATE _USE_MATH_DEFINES)
endif()
