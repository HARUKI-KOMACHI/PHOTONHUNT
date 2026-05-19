#pragma once
//// 目标 Windows 版本（Windows 7 或更高）
//#define _WIN32_WINNT 0x0601
//#include <sdkddkver.h>
//// 以下 #define 禁用了一堆未使用的 Windows 内容。
//// 如果您在尝试执行某些 Windows 内容时遇到奇怪的错误，请尝试删除部分
////（或全部）这些定义（但这会增加构建时间）。
//
//#define WIN32_LEAN_AND_MEAN      //精简Windows头文件，排除较少使用的APIs
//#define NOMINMAX                 //不包含最小最大相关的函数
//
//
//
//#define FULL_WINTARD
//
//#ifndef FULL_WINTARD             //
//
//#define NOGDICAPMASKS            //不包括GDI（图形设备接口）功能掩码
//#define NOSYSMETRICS             //不包括系统指标信息
//#define NOMENUS                  //不包括菜单相关的函数
//#define NOICONS                  //不包括图标相关的函数
//#define NOSYSCOMMANDS            //不包括系统命令相关的函数
//#define NORASTEROPS              //不包括栅格操作
//#define OEMRESOURCE              //包括OEM资源
//#define NOATOM                   //不包括原子操作函数
//#define NOCLIPBOARD              //不包括剪贴板相关的函数
//#define NOCOLOR                  //不包括颜色管理函数
//#define NOCTLMGR                 //不包括控制管理器
//#define NODRAWTEXT               //不包括绘制文本相关的函数
//#define NOKERNEL                 //不包括核心操作相关的函数
//#define NONLS                    //不包括国家语言支持（NLS）
//#define NOMEMMGR                 //不包括内存管理函数
//#define NOMETAFILE               //不包括元文件相关的函数
//#define NOOPENFILE               //不包括文件打开相关的函数
//#define NOSCROLL                 //不包括滚动条相关的函数
//#define NOSERVICE                //不包括系统服务相关的函数
//#define NOSOUND                  //不包括声音相关的函数
//#define NOTEXTMETRIC             //不包括文本度量相关的函数
//#define NOWH                     //不包括Windows钩子相关的函数
//#define NOCOMM                   //不包括通信（串行和并行）相关的函数
//#define NOKANJI                  //不包括对汉字处理相关的函数
//#define NOHELP                   //不包括帮助系统相关的函数
//#define NOPROFILER               //不包括性能分析（Profiler）相关的函数
//#define NODEFERWINDOWPOS         //不包括延迟窗口定位相关的函数
//#define NOMCX                    //不包括调制解调器配置扩展
//#define NORPC                    //不包括远程过程调用相关的函数
//#define NOPROXYSTUB              //不包括代理和存根相关的函数
//#define NOIMAGE                  //不包括图像处理相关的函数
//#define NOTAPE                   //不包括磁带驱动器相关的函数
//#endif
//
//
//#define STRICT

#include <Windows.h>
#include <iostream>
/*
* 这里主要对于windows.h的一个优化，主要是去除掉那些不用的内容
* 这样子可以减少windows.h在运行的时候的一个简化
*/