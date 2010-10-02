//
// C++ Interface: system_phrase
//
// Description:
// 根据系统码表文件数据构建词语树，并接受以汉字代理数组为参数的查询方式.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_SYSTEM_PHRASE_H_
#define PYE_ENGINE_SYSTEM_PHRASE_H_

#include "abstract_phrase.h"

/**
 * 词语树长度节点.
 */
class SystemPhraseLengthNode {
 public:
  SystemPhraseLengthNode()
      : phrase_amount_(0), index_offset_(0), chars_proxy_(NULL) {}
  ~SystemPhraseLengthNode() {
    delete [] chars_proxy_;
  }

  uint phrase_amount_;  ///< 词语总数
  int index_offset_;  ///< 相对偏移量
  CharsProxy *chars_proxy_;  ///< 汉字代理数组
};

/**
 * 词语树索引节点.
 */
class SystemPhraseIndexNode {
 public:
  SystemPhraseIndexNode() : max_length_(0), table_(NULL) {}
  ~SystemPhraseIndexNode() {
    delete [] table_;
  }

  int max_length_;  ///< 最大长度
  SystemPhraseLengthNode *table_;  ///< 索引表
};

/**
 * 词语树根节点.
 */
class SystemPhraseRootNode {
 public:
  SystemPhraseRootNode() : max_index_(-1), table_(NULL) {}
  ~SystemPhraseRootNode() {
    delete [] table_;
  }

  int8_t max_index_;  ///< 最大索引
  SystemPhraseIndexNode *table_;  ///< 索引表
};

/**
 * 系统系统查询、管理者.
 */
class SystemPhrase : public AbstractPhrase {
 public:
  SystemPhrase();
  virtual ~SystemPhrase();

  virtual void BuildPhraseTree(const char *mbfile);
  virtual void SetFuzzyPinyinTable(const int8_t **fuzzy_pair_table);
  virtual std::list<PhraseProxy *> *SearchMatchablePhrase(
                                        const CharsProxy *chars_proxy,
                                        int chars_proxy_length);
  virtual PhraseProxy *SearchPreferPhrase(const CharsProxy *chars_proxy,
                                          int chars_proxy_length);
  virtual PhraseDatum *AnalyzePhraseProxy(const PhraseProxy *phrase_proxy);

 private:
  void ReadPhraseTree();
  std::list<PhraseProxy *> *SearchMatchablePhrase(int8_t chars_proxy_index,
                                                  const CharsProxy *chars_proxy,
                                                  int chars_proxy_length);
  PhraseProxy *SearchPreferPhrase(int8_t chars_proxy_index,
                                  const CharsProxy *chars_proxy,
                                  int chars_proxy_length);

  const int8_t **fuzzy_pair_table_;  ///< 模模糊拼音单元对照表
  SystemPhraseRootNode root_;  ///< 词语树的根索引点
  int index_offset_;  ///< 绝对偏移量
  int fd_;  ///< 词语数据文件描述符
};

#endif  // PYE_ENGINE_SYSTEM_PHRASE_H_
