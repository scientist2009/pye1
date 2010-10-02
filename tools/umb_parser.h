//
// C++ Interface: umb_parser
//
// Description:
// 分析二进制的用户码表文件，并生成一份文本的词语文件.
// 词语文件格式: 词语 拼音 频率
// e.g.: 郁闷 yu'men 1234
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_TOOLS_UMB_PARSER_H_
#define PYE_TOOLS_UMB_PARSER_H_

#include <stdio.h>
#include "engine/abstract_phrase.h"

/**
 * 词语属性.
 */
class UserPhraseAttribute {
 public:
  UserPhraseAttribute() : datum_offset_(0), frequency_(0) {}
  ~UserPhraseAttribute() {}

  int datum_offset_;  ///< 词语数据的偏移量
  int frequency_;  ///< 词语的使用频率
};

/**
 * 词语树长度节点.
 */
class UserPhraseLengthNode {
 public:
  UserPhraseLengthNode()
      : phrase_amount_(0), chars_proxy_(NULL), phrase_attribute_(NULL) {}
  ~UserPhraseLengthNode() {
    delete [] chars_proxy_;
    delete [] phrase_attribute_;
  }

  uint phrase_amount_;  ///< 词语总数
  CharsProxy *chars_proxy_;  ///< 汉字代理数组
  UserPhraseAttribute *phrase_attribute_;  ///< 词语属性
};

/**
 * 词语树索引节点.
 */
class UserPhraseIndexNode {
 public:
  UserPhraseIndexNode() : max_length_(0), table_(NULL) {}
  ~UserPhraseIndexNode() {
    delete [] table_;
  }

  int max_length_;  ///< 最大长度
  UserPhraseLengthNode *table_;  ///< 索引表
};

/**
 * 词语树根节点.
 */
class UserPhraseRootNode {
 public:
  UserPhraseRootNode() : max_index_(-1), table_(NULL) {}
  ~UserPhraseRootNode() {
    delete [] table_;
  }

  int8_t max_index_;  ///< 最大索引
  UserPhraseIndexNode *table_;  ///< 索引表
};

/**
 * 用户码表分析者.
 */
class UMBParser {
 public:
  UMBParser();
  ~UMBParser();

  void BuildPhraseTree(const char *mb_file);
  void WritePhraseDatum(const char *data_file, int len, bool reset);

 private:
  void ReadPhraseTree();

  void WritePhraseTree(FILE *stream, int len, bool reset);
  void WriteDatum(FILE *stream, const PhraseProxy *phrase_proxy);
  void AnalyzePhraseProxy(const PhraseProxy *phrase_proxy,
                          PhraseDatum *phrase_datum);

  UserPhraseRootNode root_;  ///< 词语树的根索引点
  int fd_;  ///< 词语数据文件描述符
};

#endif  // PYE_TOOLS_UMB_PARSER_H_
