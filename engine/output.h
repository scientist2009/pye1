//
// C++ Interface: output
//
// Description:
// 输出宏.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_OUTPUT_H_
#define PYE_ENGINE_OUTPUT_H_

#include <config.h>
#include <err.h>
#include <stdio.h>

/* 常规消息输出 */
#ifdef MESSAGE
#define pmessage(format,...) printf(format, ##__VA_ARGS__)
#else
#define pmessage(format,...) ((void)0)
#endif

/* 警告消息输出 */
#ifdef WARNING
#define pwarning(format,...) warnx(format, ##__VA_ARGS__)
#else
#define pwarning(format,...) ((void)0)
#endif

/* 程序执行踪迹输出，用于调试 */
#ifdef TRACE
#define ptrace(format,...) printf(format, ##__VA_ARGS__)
#else
#define ptrace(format,...) ((void)0)
#endif

#endif  // PYE_ENGINE_OUTPUT_H_
