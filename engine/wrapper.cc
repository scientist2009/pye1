//
// C++ Implementation: wrapper
//
// Description:
// 参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wrapper.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

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
 * @param src 源文件
 * @param dst 目标文件
 * @return 结果值，0 成功;-1 失败
 */
int xcopy(const char *src, const char *dst) {
  int srcfd = open(src, O_RDONLY);
  if (srcfd == -1)
    return -1;
  int dstfd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 00644);
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
