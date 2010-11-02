//
// C++ Interface: phrase_manager
//
// Description:
// 词语管理者，此类管理着多个词语类，并借助它们完成词语查询的具体工作.
// 系统码表配置文件格式: 文件名 优先级
// e.g.: pinyin1.mb 18
//       pinyin2.mb 12
//       pinyin3.mb 50
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_PHRASE_MANAGER_H_
#define PYE_ENGINE_PHRASE_MANAGER_H_

#include "abstract_phrase.h"
#include "pye_global.h"

/**
 * 外部拼音纠错对.
 */
class OuterMendPinyinPair {
 public:
  OuterMendPinyinPair() : raw_(NULL), mend_(NULL) {}
  ~OuterMendPinyinPair() {
    free(raw_);
    free(mend_);
  }

  char *raw_;  ///< 原始串
  char *mend_;  ///< 纠错串
};

/**
 * 词语数据代理集合的类型.
 */
typedef enum {
  SYSTEM_TYPE,  ///< 系统
  USER_TYPE  ///< 用户
} PhraseProxySiteType;

/**
 * 词语数据代理的集合.
 */
class PhraseProxySite {
 public:
  PhraseProxySite() : phrase_(NULL), type_(SYSTEM_TYPE), priority_(0) {}
  ~PhraseProxySite() {
    delete phrase_;
  }

  AbstractPhrase *phrase_;  ///< 词语类
  PhraseProxySiteType type_;  ///< 类型
  int priority_;  ///< 优先级
};

/**
 * 词语数据代理的储存点.
 */
class PhraseProxyStorage {
 public:
  PhraseProxyStorage() : phrase_proxy_site_(NULL), phrase_proxy_list_(NULL) {}
  ~PhraseProxyStorage() {
    STL_DELETE_DATA(*phrase_proxy_list_, std::list<PhraseProxy *>);
    delete phrase_proxy_list_;
  }

  const PhraseProxySite *phrase_proxy_site_;  ///< 词语数据代理的集合
  std::list<PhraseProxy *> *phrase_proxy_list_;  ///< 词语数据代理的链表
};

/**
 * 词语管理者.
 */
class PhraseManager {
 public:
  /* 外部接口 */
  void CreateSystemPhraseProxySite(const char *config);
  void CreateUserPhraseProxySite(const char *mbfile);
  void AppendMendPinyinPair(const char *raw, const char *mend);
  void AppendFuzzyPinyinPair(const char *unit1, const char *unit2);
  void ClearMendPinyinPair();
  void ClearFuzzyPinyinPair();
  void BackupUserPhrase();

  void DeletePhraseDatum(const PhraseDatum *phrase_datum) const;
  void FeedbackPhraseDatum(const PhraseDatum *phrase_datum) const;
  std::list<PhraseProxyStorage *> *SearchMatchablePhrase(
                                       const CharsProxy *chars_proxy,
                                       int chars_proxy_length) const;
  PhraseProxyStorage *SearchPreferPhrase(const CharsProxy *chars_proxy,
                                         int chars_proxy_length) const;
  const std::list<OuterMendPinyinPair *> *GetMendPinyinTable() const;

  static PhraseManager *GetInstance();

 private:
  PhraseManager();
  ~PhraseManager();

  bool BreakMbfileString(char *string, const char **mbfile,
                         const char **priority);
  PhraseProxySite *CreatePhraseProxySite(const char *mbfile, int priority,
                                         PhraseProxySiteType type);

  std::list<PhraseProxySite *> phrase_proxy_site_list_;  ///< 集合链表
  std::list<OuterMendPinyinPair *> mend_pair_table_;  ///< 拼音矫正表
  int8_t **fuzzy_pair_table_;  ///< 模糊对照表

  char *user_path_;  ///< 用户码表路径
  char *backup_path_;  ///< 备份码表路径
};

#endif  // PYE_ENGINE_PHRASE_MANAGER_H_
