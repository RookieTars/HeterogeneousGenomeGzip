cmake_minimum_required(VERSION 3.0.0)
project(gzip_cpu VERSION 0.1.0)

# 开O3优化
set(CMAKE_BUILD_TYPE "Release")

# 设定目标系统，必须设定
set(CMAKE_SYSTEM_NAME Android)

# C 编译器，必须设定

set(CMAKE_C_COMPILER "/home/wxt/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang")

# C++ 编译器，必须设定

set(CMAKE_CXX_COMPILER "/home/wxt/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang++")


set(CMAKE_SYSROOT "/home/wxt/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/sysroot")

# “30”代表Android ABI的版本
# "-fopenmp"指链接openmp
add_compile_options(--target=aarch64-unknown-linux-android30 -fopenmp)

# 这是我放额外的特殊头文件的地方
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 这是我放需要链接的lib的地方
# LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/aarch64)

# C 版本
set(CMAKE_CXX_STANDARD 20)

# C++ 版本
set(CMAKE_C_STANDARD 20)

# 预定义宏，相当于 #define CL_TARGET_OPENCL_VERSION 200
add_definitions(-DCL_TARGET_OPENCL_VERSION=200)

file(GLOB_RECURSE SRC_DIR_LIST "src/*.cpp" "src/*.hpp" "src/*.h" "src/*.cc")
# 生成可执行文件
add_executable(${CMAKE_PROJECT_NAME} ${SRC_DIR_LIST})
