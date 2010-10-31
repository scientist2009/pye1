//
// C++ Interface: umb_creater
//
// Description:
// 分析词语文件，并生成一份二进制的用户码表文件.
// 词语文件格式: 词语 拼音 频率
// e.g.: 郁闷 yu'men 1234
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_TOOLS_UMB_CREATER_H_
#define PYE_TOOLS_UMB_CREATER_H_

#include <sys/types.h>
#include <stdlib.h>
#include <list>
#include "engine/pinyin_parser.h"
#include "engine/global.h"

/**
 * 词语数据资料.
 */
class PhraseDatum {
 public:
  PhraseDatum()
      : chars_proxy_(NULL), chars_proxy_length_(0),
        raw_data_(NULL), raw_data_length_(0),
        frequency_(0) {}
  ~PhraseDatum() {
    delete [] chars_proxy_;
    free(raw_data_);
  }

  CharsProxy *chars_proxy_;  ///< 词语的汉字代理数组 *
  int chars_proxy_length_;  ///< 词语的汉字代理数组的长度
  void *raw_data_;  ///< 词语的原始数据 *
  int raw_data_length_;  ///< 词语的原始数据的长度
  int frequency_;  ///< 词语的使用频率
};

/**
 * 词语树长度节点.
 */
class PhraseLengthNode {
 public:
  PhraseLengthNode() : chars_proxy_length_(0) {}
  ~PhraseLengthNode() {
    STL_DELETE_DATA(data_, std::list<PhraseDatum *>);
  }

  int chars_proxy_length_;  ///< 词语的汉字代理数组的长度
  std::list<PhraseDatum *> data_;  ///< 数据
};

/**
 * 词语树索引节点.
 */
class PhraseIndexNode {
 public:
  PhraseIndexNode() : chars_proxy_index_(-1) {}
  ~PhraseIndexNode() {
    STL_DELETE_DATA(data_, std::list<PhraseLengthNode *>);
  }

  int8_t chars_proxy_index_;  ///< 词语的汉字代理数组的索引
  std::list<PhraseLengthNode *> data_;  ///< 数据
};

/**
 * 词语树根节点.
 */
class PhraseRootNode {
 public:
  PhraseRootNode() {}
  ~PhraseRootNode() {
    STL_DELETE_DATA(data_, std::list<PhraseIndexNode *>);
  }

  std::list<PhraseIndexNode *> data_;  ///< 数据
};

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
 * 用户码表创建者.
 */
class UMBCreater {
 public:
  UMBCreater();
  ~UMBCreater();

  void BuildPhraseTree(const char *data_file);
  void WritePhraseTree(const char *mb_file);

 private:
  bool BreakPhraseString(char *string, const char **phrase,
                         const char **pinyin, const char **frequency);
  PhraseDatum *CreatePhraseDatum(const char *phrase, const char *pinyin,
                                 const char *frequency);
  void InsertDatumToTree(PhraseDatum *datum);
  std::list<PhraseLengthNode *> *SearchChildByIndex(
      std::list<PhraseIndexNode *> *data_list, int index);
  std::list<PhraseDatum *> *SearchChildByLength(
      std::list<PhraseLengthNode *> *data_list, int length);

  int RebuildPhraseTree(int fd);
  void WritePhraseTree(int fd, int offset);

  PhraseRootNode root_;  ///< 词语树的根节点
  UserPhraseRootNode user_root_;  ///< 词语树的根索引点
};

#endif  // PYE_TOOLS_UMB_CREATER_H_
