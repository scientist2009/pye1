//
// C++ Interface: user_phrase
//
// Description:
// 根据用户码表文件数据构建词语树，并接受以汉字代理数组为参数的查询方式.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_USER_PHRASE_H_
#define PYE_ENGINE_USER_PHRASE_H_

#include "abstract_phrase.h"

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
 * 用户词语管理、查询者.
 */
class UserPhrase : public AbstractPhrase {
 public:
  UserPhrase();
  virtual ~UserPhrase();

  virtual void BuildPhraseTree(const char *mbfile);
  virtual void SetFuzzyPinyinTable(const int8_t **fuzzy_pair_table);
  virtual std::list<PhraseProxy *> *SearchMatchablePhrase(
                                        const CharsProxy *chars_proxy,
                                        int chars_proxy_length);
  virtual PhraseProxy *SearchPreferPhrase(const CharsProxy *chars_proxy,
                                          int chars_proxy_length);
  virtual PhraseDatum *AnalyzePhraseProxy(const PhraseProxy *phrase_proxy);

  void InsertPhraseToTree(const PhraseDatum *phrase_datum);
  void DeletePhraseFromTree(const PhraseDatum *phrase_datum);
  void IncreasePhraseFrequency(const PhraseDatum *phrase_datum);
  void WritePhraseTree();

private:
  void WriteEmptyPhraseTree();

  void ReadPhraseTree();
  std::list<PhraseProxy *> *SearchMatchablePhrase(int8_t chars_proxy_index,
                                                  const CharsProxy *chars_proxy,
                                                  int chars_proxy_length);
  PhraseProxy *SearchPreferPhrase(int8_t chars_proxy_index,
                                  const CharsProxy *chars_proxy,
                                  int chars_proxy_length);

  const int8_t **fuzzy_pair_table_;  ///< 模模糊拼音单元对照表
  UserPhraseRootNode root_;  ///< 词语树的根索引点
  int index_offset_;  ///< 绝对偏移量
  int fd_;  ///< 词语数据文件描述符
};

#endif
