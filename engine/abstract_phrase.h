//
// C++ Interface: abstract_phrase
//
// Description:
// 抽象词语查询、管理者.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_ABSTRACT_PHRASE_H_
#define PYE_ENGINE_ABSTRACT_PHRASE_H_

#include <sys/types.h>
#include <stdlib.h>
#include <list>
#include "pinyin_parser.h"

/**
 * 词语数据代理.
 */
class PhraseProxy {
 public:
  PhraseProxy()
      : chars_proxy_(NULL), chars_proxy_length_(0),
        phrase_data_offset_(0), frequency_(0) {}
  ~PhraseProxy() {}

  const CharsProxy *chars_proxy_;  ///< 词语的汉字代理数组 *
  int chars_proxy_length_;  ///< 词语的汉字代理数组的长度
  int phrase_data_offset_;  ///< 词语数据的偏移量
  int frequency_;  ///< 词语的使用频率
};

/* 偏移量的特殊含义 */
#define EnginePhraseType -3
#define ManualPhraseType -2
#define SystemPhraseType -1
#define InvalidPhraseType 0
#define UserPhrasePoint 1
/**
 * 词语数据资料.
 */
class PhraseDatum {
 public:
  PhraseDatum()
      : chars_proxy_(NULL), chars_proxy_length_(0),
        raw_data_(NULL), raw_data_length_(0),
        phrase_data_offset_(0) {}
  ~PhraseDatum() {
    delete [] chars_proxy_;
    free(raw_data_);
  }

  CharsProxy *chars_proxy_;  ///< 词语的汉字代理数组 *
  int chars_proxy_length_;  ///< 词语的汉字代理数组的长度
  void *raw_data_;  ///< 词语的原始数据 *
  int raw_data_length_;  ///< 词语的原始数据的长度
  int phrase_data_offset_;  ///< 词语数据的偏移量(特殊含义)
};

/**
 * 抽象词语查询、管理者.
 */
class AbstractPhrase {
 public:
  AbstractPhrase() {}
  virtual ~AbstractPhrase() {}

  virtual void BuildPhraseTree(const char *mbfile) = 0;
  virtual void SetFuzzyPinyinTable(const int8_t **fuzzy_pair_table) = 0;
  virtual std::list<PhraseProxy *> *SearchMatchablePhrase(
                                        const CharsProxy *chars_proxy,
                                        int chars_proxy_length) = 0;
  virtual PhraseProxy *SearchPreferPhrase(const CharsProxy *chars_proxy,
                                          int chars_proxy_length) = 0;
  virtual PhraseDatum *AnalyzePhraseProxy(const PhraseProxy *phrase_proxy) = 0;

 protected:
  /**
   * 检查两个汉字代理数组是否相匹配.
   * @param table 对照表
   * @param dst 目标
   * @param src 源
   * @param len 长度
   * @return BOOL
   */
  bool CharsProxyCmp(const int8_t **table,
                     const CharsProxy *dst,
                     const CharsProxy *src,
                     int len) {
    int count = 0;
    for (; count < len; ++count) {
      /* 主部件 */
      int8_t di = (dst + count)->major_index_;
      const int8_t *sip = *(table + (src + count)->major_index_);
      for (; *sip != -1; ++sip) {
        if (*sip == di)
          break;
      }
      if (*sip == -1)
        break;
      /* 副部件 */
      if ((di = (dst + count)->minor_index_) == -1)
        continue;
      int8_t si = (src + count)->minor_index_;
      if (si == -1)
        break;
      sip = *(table + si);
      for (; *sip != -1; ++sip) {
        if (*sip == di)
          break;
      }
      if (*sip == -1)
        break;
    }
    return count == len;
  }
};

#endif  // PYE_ENGINE_ABSTRACT_PHRASE_H_
