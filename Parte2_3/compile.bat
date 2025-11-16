@echo off
echo Compiling Golomb Coding projects...
echo.

echo [1/2] Compiling Golomb Test...
g++ -std=c++11 -D_USE_MATH_DEFINES -o golomb_test.exe main.cpp GolombCoding.cpp
if %errorlevel% neq 0 (
    echo ERROR: Golomb test compilation failed!
    exit /b 1
)
echo       Success!

echo [2/2] Compiling Audio Codec Test...
g++ -std=c++11 -D_USE_MATH_DEFINES -o audio_test.exe audio_test.cpp AudioCodec.cpp WAVFile.cpp GolombCoding.cpp
if %errorlevel% neq 0 (
    echo ERROR: Audio test compilation failed!
    exit /b 1
)
echo       Success!

echo.
echo ================================================
echo Compilation completed successfully!
echo ================================================
echo.
echo Run './golomb_test.exe' to test Golomb coding
echo Run './audio_test.exe' to test audio codec
echo.
