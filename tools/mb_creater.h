//
// C++ Interface: mb_creater
//
// Description:
// 分析词语文件，并生成一份二进制的系统码表文件.
// 词语文件格式: 词语 拼音 频率
// e.g.: 郁闷 yu'men 1234
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_TOOLS_MB_CREATER_H_
#define PYE_TOOLS_MB_CREATER_H_

#include <sys/types.h>
#include <stdlib.h>
#include <list>
#include "engine/global.h"
#include "engine/pinyin_parser.h"

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
 * 系统码表创建者.
 */
class MBCreater {
 public:
  MBCreater();
  ~MBCreater();

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

  uint WritePureIndexPart(int fd, int *offset);
  void WriteDatumIndexPart(int fd, int offset);
  void WritePhraseDatumPart(int fd);

  PhraseRootNode root_;  ///< 词语树的根节点
};

#endif  // PYE_TOOLS_MB_CREATER_H_
