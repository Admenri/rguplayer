# ![Logo](app/resources/rgu_favicon_64.png) RGU Player Core

## 项目概述

RGU是一款兼容RGSS 1/2/3，使用SDL作为底层，
OpenGL ES(2.0) 图像标准编写渲染部分的异步多线程2D游戏引擎。
旨在提供兼容RGSS的同时提供跨平台与性能提升支持。

本项目使用BSD-3协议开源。

本项目语法风格与代码结构与The Chromium Project相似。

## 编译项目

 - 本项目使用CMake管理编译。
 - 第三方依赖库部分使用Git拉取，部分需要用户自行编译处理。
 - 项目中需要使用Python3来生成自动编译文件，请确保系统中已安装Python3。

※ 开发中暂不提供编译步骤。

## 系统要求

- Windows 7及以上
- Linux 发行版 (具体依赖发行版的内核版本)
- Android 5.0及以上
- 其他平台支持计划中

## 思路来源

- SDL
- RGM
- MKXP
- SFML
- Chromium

## 第三方库使用

- Chromium - https://www.chromium.org/chromium-projects/
- SDL - https://github.com/libsdl-org/SDL
- SDL_image - https://github.com/libsdl-org/SDL_image
- SDL_ttf - https://github.com/libsdl-org/SDL_ttf
- ANGLE - https://chromium.googlesource.com/angle/angle
- CRuby - https://www.ruby-lang.org/zh_cn/
- Physfs - https://github.com/icculus/physfs
- MRuby - https://mruby.org/
- concurrentqueue - https://github.com/cameron314/concurrentqueue
- aom - https://aomedia.googlesource.com/aom

## 联系方式

- AFDian: https://afdian.net/a/rguplayer
- Mail: 2755482106@qq.com

© 2024 Admenri