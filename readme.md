### 面向移动设备的 GZIP 压缩算法说明文档

#### 介绍

​ GZIP 压缩算法的基本原理是：基于对数据进行重复字符串的替换和编码，以实现数据压缩的目的，由 LZ77 压缩和 Huffman 压缩两部分组成。基本流程为：对于要压缩的文件，首先使用 LZ77 算法的一个变种进行压缩，通过寻找数据中的重复模式，以替代其中的一些数据。对得到的结果再使用 Huffman 编码(根据情况，使用静态 Huffman 编码或动态 Huffman 编码)，将出现频率较高的字符串编码为较短的编码，而出现频率较低的字符串则编码为较长的编码。

​ 本项目分别实现了单核 CPU 和 CPU 大小核流水线的压缩代码，并进行了测试。其中，大小核 CPU 流水线代码中，将 LZ77 放进超大核中，将 huffman 压缩过程放进小核中。

#### 开发环境

1. windows 10 + Visutal Studio Code+wsl2 + Ubuntu 20.04 LTS
2. android-ndk-r25b
3. 在 Google pixel 4 上进行测试

#### 测试数据集

测试数据集已上传百度网盘。

分享地址：https://pan.baidu.com/s/19MpKdUUCARFQhIWmXM02uQ?pwd=qqrs
提取码：qqrs

#### 使用说明

##### 配置 ndk 环境

1. 在官网https://developer.android.google.cn/ndk/downloads?hl=zh-cn 下载`Android NDK`，测试使用的版本为：`android-ndk-r25b`

2. 将环境变量`ANDROID_NDK_HOME`指向`NDK`目录

    ```
    export ANDROID_NDK_HOME=/path/to/your_android_ndk_root
    ```

##### 编译生成可执行程序

​ 修改`CMakeLists.txt`中对应路径后，使用以下语句编译生成可执行程序：

```
cd /path/to/aes

mkdir build
cd build
cmake ..
make
```

##### 执行程序

1.将生成的可执行程序发往移动端

```
adb push GZIP /data/local/tmp
```

2.在移动端执行

```
adb shell su -c /data/local/tmp/GZIP
```

如果出现缺失相关的依赖，按照下列操作执行

-   缺少`libc++_shared.so`，运行

```
adb push ${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so /data/local/tmp
```

-   缺少`libomp.so`，运行

```
adb push ${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64/lib64/clang/14.0.6/lib/linux/aarch64/libomp.so /data/local/tmp
```

-   缺少`libOpenCL.so`，运行

```
cd /path/to/gzip

adb push libs/libOpenCL.so /data/local/tmp
```

将相关依赖库置于`/data/local/tmp`目录下后，执行：

```
adb shell "export LD_LIBRARY_PATH=/data/local/tmp/:$LD_LIBRARY_PATH;su -c /data/local/tmp/GZIP;"
```

##### 具体使用区别

-   单核 CPU 版本的代码按照上述步骤执行后，会提示输入待压缩文件的路径和名称，压缩完成的结果文件以`.gzip`的形式存储在与输入路径相同的文件夹下。

-   大小核 CPU 版本的代码需要将待压缩文件的路径和名称，在执行程序时一并传入。

    ```
    e.g GZIP /path/to/inputfile
    ```
