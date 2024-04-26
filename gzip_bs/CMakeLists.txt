cmake_minimum_required(VERSION 3.0.0)
project(gzip_bs_energy VERSION 0.1.0)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_SYSTEM_NAME Android)

set(CMAKE_C_COMPILER "/home/wxt/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang")

set(CMAKE_CXX_COMPILER "/home/wxt/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang++")

set(CMAKE_SYSROOT "/home/wxt/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/sysroot")

add_compile_options(--target=aarch64-unknown-linux-android30 -fopenmp)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_STANDARD 20)

add_definitions(-DCL_TARGET_OPENCL_VERSION=200)

file(GLOB_RECURSE SRC_DIR_LIST "src/*.cpp" "src/*.hpp" "src/*.h" "src/*.cc")
add_executable(${CMAKE_PROJECT_NAME} ${SRC_DIR_LIST})
