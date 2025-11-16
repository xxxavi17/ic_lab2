# Parte4/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ImageCodecParte4)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(image_codec 
    main.cpp
    ImageCodec.cpp
    BitStream.cpp
    GolombCoding.cpp
)

target_link_libraries(image_codec ${OpenCV_LIBS})