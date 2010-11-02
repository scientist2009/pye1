//
// C++ Implementation: pye_wrapper
//
// Description:
// 参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pye_wrapper.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/**
 * 移除串首部的空白字符.
 * @param s 串
 * @return 新串
 */
char *strchug(char *s) {
  const char *ptr = s;
  for (; isspace(*ptr); ++ptr)
    continue;
  if (ptr != s) {
    char *pptr = s;
    for (; *ptr != '\0'; ++pptr, ++ptr)
      *pptr = *ptr;
    *pptr = '\0';
  }
  return s;
}

/**
 * 移除串尾部的空白字符.
 * @param s 串
 * @return 新串
 */
char *strchomp(char *s) {
  char *ptr = s + strlen(s) - 1;
  for (; ptr >= s && isspace(*ptr); --ptr)
    continue;
  if (ptr != s)
    *(ptr + 1) = '\0';
  return s;
}

/**
 * 写出数据.
 * @param fd as in write()
 * @param buf as in write()
 * @param count as in write()
 * @return 成功写出的字节数
 */
ssize_t xwrite(int fd, const void *buf, size_t count) {
  ssize_t size = -1;
  size_t offset = 0;
  while ((offset != count) && (size != 0)) {
    if ((size = write(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR)
        continue;
      return -1;
    }
    offset += size;
  }

  return offset;
}

/**
 * 读取数据.
 * @param fd as in read()
 * @param buf as in read()
 * @param count as in read()
 * @return 成功读取的字节数
 */
ssize_t xread(int fd, void *buf, size_t count) {
  ssize_t size = -1;
  size_t offset = 0;
  while ((offset != count) && (size != 0)) {
    if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR)
        continue;
      return -1;
    }
    offset += size;
  }

  return offset;
}

/**
 * 拷贝文件.
 * @param srcfile 源文件
 * @param dstfile 目标文件
 * @return 结果值，0 成功;-1 失败
 */
int xcopy(const char *srcfile, const char *dstfile) {
  int srcfd = open(srcfile, O_RDONLY);
  if (srcfd == -1)
    return -1;
  int dstfd = open(dstfile, O_WRONLY | O_CREAT | O_TRUNC, 00644);
  if (dstfd == -1) {
    close(srcfd);
    return -1;
  }

  int result = 0;
  while (1) {
    char buffer[4096];
    ssize_t size = xread(srcfd, buffer, sizeof(buffer));
    if (size == -1) {
      result = -1;
      break;
    }
    if (size == 0)
      break;
    if (xwrite(dstfd, buffer, size) != size) {
      result = -1;
      break;
    }
  }
  close(srcfd);
  close(dstfd);

  return result;
}
