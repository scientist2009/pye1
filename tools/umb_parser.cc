//
// C++ Implementation: umb_parser
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "umb_parser.h"
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "engine/wrapper.h"

/**
 * 类构造函数.
 */
UMBParser::UMBParser() : fd_(-1) {
}

/**
 * 类析构函数.
 */
UMBParser::~UMBParser() {
  close(fd_);
}

/**
 * 根据用户码表文件构建词语树.
 * @param mb_file 用户码表文件
 */
void UMBParser::BuildPhraseTree(const char *mb_file) {
  /* 打开码表文件 */
  if ((fd_ = open(mb_file, O_RDONLY)) == -1)
    errx(1, "Open file \"%s\" failed, %s", mb_file, strerror(errno));

  /* 读取词语索引 */
  ReadPhraseTree();
}

/**
 * 以文本文件的方式写出词语树.
 * @param data_file 目标文件
 * @param len 短语有效长度
 * @param reset 是否重置短语频率
 */
void UMBParser::WritePhraseDatum(const char *data_file, int len, bool reset) {
  /* 打开输出文件 */
  FILE *stream = fopen(data_file, "wb");
  if (!stream)
    errx(1, "Fopen file \"%s\" failed, %s", data_file, strerror(errno));

  /* 写出词语树 */
  WritePhraseTree(stream, len, reset);
  /* 关闭文件 */
  fclose(stream);
}

/**
 * 读取码表文件的索引部分，并建立词语树.
 */
void UMBParser::ReadPhraseTree() {
  /* 读取索引偏移量 */
  int index_offset = 0;
  lseek(fd_, 0, SEEK_SET);
  xread(fd_, &index_offset, sizeof(index_offset));
  /* 读取根节点的数据 */
  lseek(fd_, index_offset, SEEK_SET);
  xread(fd_, &root_, sizeof(UserPhraseRootNode));
  /* 读取索引节点的数据 */
  if (root_.max_index_ == -1)
    return;
  root_.table_ = new UserPhraseIndexNode[root_.max_index_ + 1];
  xread(fd_, root_.table_,
        sizeof(UserPhraseIndexNode) * (root_.max_index_ + 1));
  /* 读取长度节点的数据 */
  for (int8_t index = 0; index <= root_.max_index_; ++index) {
    UserPhraseIndexNode *index_node = root_.table_ + index;
    if (index_node->max_length_ == 0)
      continue;
    index_node->table_ = new UserPhraseLengthNode[index_node->max_length_];
    xread(fd_, index_node->table_,
          sizeof(UserPhraseLengthNode) * index_node->max_length_);
    /* 读取词语数据资料 */
    for (int length = 1; length <= index_node->max_length_; ++length) {
      UserPhraseLengthNode *length_node = index_node->table_ + length - 1;
      if (length_node->phrase_amount_ == 0)
        continue;
      length_node->chars_proxy_ =
          new CharsProxy[length * length_node->phrase_amount_];
      xread(fd_, length_node->chars_proxy_,
            sizeof(CharsProxy) * length * length_node->phrase_amount_);
      length_node->phrase_attribute_ =
          new UserPhraseAttribute[length_node->phrase_amount_];
      xread(fd_, length_node->phrase_attribute_,
            sizeof(UserPhraseAttribute) * length_node->phrase_amount_);
    }
  }
}

/**
 * 写出词语树.
 * @param stream 输出流
 * @param len 短语有效长度
 * @param reset 是否重置短语频率
 */
void UMBParser::WritePhraseTree(FILE *stream, int len, bool reset) {
  UserPhraseRootNode *root_node = &root_;
  for (int8_t index = 0; index <= root_node->max_index_; ++index) {
    UserPhraseIndexNode *index_node = root_node->table_ + index;
    for (int length = len; length <= index_node->max_length_; ++length) {
      UserPhraseLengthNode *length_node = index_node->table_ + length - 1;
      for (uint number = 0; number < length_node->phrase_amount_; ++number) {
        PhraseProxy phrase_proxy;
        phrase_proxy.chars_proxy_ = length_node->chars_proxy_ + length * number;
        phrase_proxy.chars_proxy_length_ = length;
        UserPhraseAttribute *phrase_attribute =
            length_node->phrase_attribute_ + number;
        phrase_proxy.phrase_data_offset_ = phrase_attribute->datum_offset_;
        phrase_proxy.frequency_ = reset ? 0: phrase_attribute->frequency_;
        WriteDatum(stream, &phrase_proxy);
      }
    }
  }
}

/**
 * 写出词语数据.
 * @param stream 输出流
 * @param phrase_proxy 词语代理
 */
void UMBParser::WriteDatum(FILE *stream, const PhraseProxy *phrase_proxy) {
  PhraseDatum phrase_datum;
  AnalyzePhraseProxy(phrase_proxy, &phrase_datum);
  PinyinParser pinyin_parser;
  char *pinyin = pinyin_parser.UnparsePinyin(phrase_proxy->chars_proxy_,
                                             phrase_proxy->chars_proxy_length_);
  fwrite(phrase_datum.raw_data_, 1, phrase_datum.raw_data_length_, stream);
  fprintf(stream, "\t%s\t%d\n", pinyin, phrase_proxy->frequency_);
  free(pinyin);
}

/**
 * 分析词语代理所表达的词语数据.
 * @param phrase_proxy 词语代理
 * @param phrase_datum 词语数据
 */
void UMBParser::AnalyzePhraseProxy(const PhraseProxy *phrase_proxy,
                                   PhraseDatum *phrase_datum) {
  lseek(fd_, phrase_proxy->phrase_data_offset_, SEEK_SET);
  xread(fd_, &phrase_datum->raw_data_length_,
        sizeof(phrase_datum->raw_data_length_));
  phrase_datum->raw_data_ = malloc(phrase_datum->raw_data_length_);
  xread(fd_, phrase_datum->raw_data_, phrase_datum->raw_data_length_);
}
