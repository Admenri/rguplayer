# ![Logo](app/resources/rgu_favicon_64.png) RGU Player Core

## 项目概述

- RGU是一款兼容RGSS 1/2/3，使用SDL3作为底层，OpenGL ES(2.0) 图像标准编写渲染部分的异步多线程2D游戏引擎。
- RGU在提供兼容原版RGSS的同时提供跨平台与性能提升支持，同时提供诸如自定义着色器与网络扩展在内的加强功能。
- 本项目使用BSD-3协议开源。
- 本项目语法风格与代码结构与The Chromium Project相似。

## 项目结构

- 运行结构为多线程架构，程序内存在多个线程worker，每个worker都有任务投递的接口，引擎将事件处理，逻辑渲染处理，音频播放处理，视频解码处理，网络处理等分解为多个线程。

- 源码结构分为逻辑实现，图像渲染实现，基础库实现，封装组件实现与脚本引擎绑定实现，
- 整个程序的入口在app文件夹中
- content文件夹中存放负责组织引擎全部内容（图像，输入，音频）功能的代码，是引擎的核心实现
- components中存放引擎中的某些特定组件实现（如IO系统，fps计数器等）
- base文件夹中存放跨平台的基础代码
- binding文件夹中存放与cruby，mruby等第三方解释器进行绑定的代码
- buildtools中存放所有的python自动化代码
- renderer文件夹存放了GLES2.0渲染器的初级封装代码
- third_party中为使用的第三方代码库，base/third_party中也有部分第三方库
- ui文件夹存放了SDL窗口的封装代码，用于与input模块进行配合操作

- 游戏的图像渲染采用OpenGL ES(2.0)标准以获得最大兼容
- 用户可以选择使用ANGLE运行其他渲染器后端（D3D9 D3D11 Vulkan Metal 软渲染等）以应对显卡驱动兼容问题
- 引擎的输入处理基于SDL的事件处理
- 引擎的音频处理基于SoLoud库，音频数据通过Soloud核心处理后输出到SDL的音频设备接口
- 引擎的脚本处理部分使用了Ruby 3.2.2的解释器

## 截图

<img src="app/test/1.png" height="300">

<img src="app/test/2.png" height="300">

<img src="app/test/3.png" height="300">

<img src="app/test/4.png" height="300">

<img src="app/test/5.jpg" height="300">

## 编译项目

 - 本项目使用CMake管理编译。
 - 第三方依赖库部分使用Git拉取，部分需要用户自行编译处理。
 - 项目中需要使用Python3来生成自动编译文件，请确保系统中已安装Python3。

### Windows (Visual Studio 2019)
 - 可以直接使用vs内置的cmake功能进行快捷构建

### Linux (Ubuntu23.10)
 - 首先通过git clone拉取项目源码: git clone https://github.com/Admenri/rguplayer.git
 - 接着git clone在third_party中用到的第三方库源码: git submodule init | git submodule update
 - 注意SDL_ttf中还有freetype
 - 然后保证系统安装了opengl开发库和ruby开发库
 - 在目录执行：cmake -S . -B out 以生成工程
 - 之后执行cmake --build out执行构建

## 系统支持

- Microsoft Windows 7 later
- Linux 6.5.0 later
- Android 4.0 later
- 其他平台支持计划中

## 思路来源

- Chromium
- RGM
- MKXP
- SDL

## 第三方库使用

- SDL - https://github.com/libsdl-org/SDL
- SDL_image - https://github.com/libsdl-org/SDL_image
- SDL_ttf - https://github.com/libsdl-org/SDL_ttf
- SDL_net - https://github.com/libsdl-org/SDL_net
- ANGLE - https://chromium.googlesource.com/angle/angle
- concurrentqueue - https://github.com/cameron314/concurrentqueue
- CRuby - https://www.ruby-lang.org/zh_cn/
- Physfs - https://github.com/icculus/physfs
- zlib - https://github.com/madler/zlib
- inih - https://github.com/jtilly/inih
- soloud - https://github.com/jarikomppa/soloud

## 联系方式

- AFDian: https://afdian.net/a/rguplayer
- Mail: 2755482106@qq.com

© 2015-2024 Admenri