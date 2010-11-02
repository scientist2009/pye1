//
// C++ Interface: pye_wrapper
//
// Description:
// 函数封装.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_PYE_WRAPPER_H_
#define PYE_ENGINE_PYE_WRAPPER_H_

#include <sys/types.h>

/* removes leading & trailing spaces */
#define strstrip(s) strchug(strchomp(s))
/* removes leading spaces */
char *strchug(char *s);
/* removes trailing spaces */
char *strchomp(char *s);

ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
int xcopy(const char *srcfile, const char *dstfile);

#endif  // PYE_ENGINE_PYE_WRAPPER_H_
