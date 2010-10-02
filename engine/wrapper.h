//
// C++ Interface: wrapper
//
// Description:
// 函数封装.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_WRAPPER_H_
#define PYE_ENGINE_WRAPPER_H_

#include <sys/types.h>

ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
int xcopy(const char *src, const char *dst);

#endif  // PYE_ENGINE_WRAPPER_H_
