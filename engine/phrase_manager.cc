//
// C++ Implementation: phrase_manager
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#define __STDC_LIMIT_MACROS
#include "phrase_manager.h"
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "output.h"
#include "system_phrase.h"
#include "user_phrase.h"
#include "wrapper.h"

/**
 * 创建系统词语数据代理集合.
 * @param config 系统码表配置文件
 */
void PhraseManager::CreateSystemPhraseProxySite(const char *config) {
  /* 打开系统码表配置文件 */
  FILE *stream = fopen(config, "r");
  if (!stream) {
    pwarning("Fopen file \"%s\" failed, %s", config, strerror(errno));
    return;
  }

  /* 获取路径 */
  char *path = strdup(config);
  const char *dir = dirname(path);

  /* 读取文件数据、分析并创建词语集合添加到链表 */
  char *lineptr = NULL;
  size_t n = 0;
  while (getline(&lineptr, &n, stream) != -1) {
    const char *filename(NULL), *priority(NULL);
    if (!BreakMbfileString(lineptr, &filename, &priority))
      continue;
    char *mbfile = NULL;
    asprintf(&mbfile, "%s/%s", dir, filename);
    PhraseProxySite *phrase_proxy_site =
        CreatePhraseProxySite(mbfile, atoi(priority), SYSTEM_TYPE);
    phrase_proxy_site_list_.push_back(phrase_proxy_site);
    free(mbfile);
  }
  free(lineptr);

  /* 释放资源 */
  free(path);
  fclose(stream);
}

/**
* 创建用户词语数据代理集合.
 * @param mbfile 用户码表文件
 */
void PhraseManager::CreateUserPhraseProxySite(const char *mbfile) {
  /* 如果用户词语数据代理集合已经存在，则直接退出即可 */
  if (user_path_)
    return;

  /* 创建运行环境 */
  user_path_ = strdup(mbfile);
  char *path = strdup(mbfile);
  const char *dir = dirname(path);
  asprintf(&backup_path_, "%s/bak.mb", dir);
  unlink(backup_path_);  // 删除可能错误的文件
  xcopy(user_path_, backup_path_);
  free(path);

  /* 创建词语数据代理集合 */
  PhraseProxySite *phrase_proxy_site =
      CreatePhraseProxySite(backup_path_, INT32_MAX, USER_TYPE);
  phrase_proxy_site_list_.push_back(phrase_proxy_site);
}

/**
 * 添加拼音矫正对.
 * @param raw 原始拼音串
 * @param mend 校正拼音串
 */
void PhraseManager::AppendMendPinyinPair(const char *raw, const char *mend) {
  OuterMendPinyinPair *pair = new OuterMendPinyinPair;
  mend_pair_table_.push_back(pair);
  pair->raw_ = strdup(raw);
  pair->mend_ = strdup(mend);
}

/**
 * 添加模糊拼音对.
 * @param unit1 拼音单元
 * @param unit2 拼音单元
 */
void PhraseManager::AppendFuzzyPinyinPair(const char *unit1,
                                          const char *unit2) {
  /* 获取索引值 */
  PinyinParser pinyin_parser;
  int8_t index1 = pinyin_parser.GetPinyinUnitPartsIndex(unit1);
  int8_t index2 = pinyin_parser.GetPinyinUnitPartsIndex(unit2);
  if (index1 == -1 || index2 == -1)
    return;

  /* 更新第一个 */
  size_t number = 0;
  int8_t *indexptr = *(fuzzy_pair_table_ + index1);
  for (; *(indexptr + number) != -1; ++number)
    continue;
  *(fuzzy_pair_table_ + index1) =
      (int8_t *)malloc(sizeof(int8_t) * (number + 2));
  memcpy(*(fuzzy_pair_table_ + index1), indexptr, sizeof(int8_t) * number);
  free(indexptr);
  indexptr = *(fuzzy_pair_table_ + index1);
  *(indexptr + number) = index2;
  *(indexptr + number + 1) = -1;

  /* 更新第二个 */
  indexptr = *(fuzzy_pair_table_ + index2);
  for (number = 0; *(indexptr + number) != -1; ++number)
    continue;
  *(fuzzy_pair_table_ + index2) =
      (int8_t *)malloc(sizeof(int8_t) * (number + 2));
  memcpy(*(fuzzy_pair_table_ + index2), indexptr, sizeof(int8_t) * number);
  free(indexptr);
  indexptr = *(fuzzy_pair_table_ + index2);
  *(indexptr + number) = index1;
  *(indexptr + number + 1) = -1;
}

/**
 * 清空拼音矫正对.
 */
void PhraseManager::ClearMendPinyinPair() {
  STL_DELETE_DATA(mend_pair_table_, std::list<OuterMendPinyinPair *>);
  mend_pair_table_.clear();
}

/**
 * 清空模糊拼音对.
 */
void PhraseManager::ClearFuzzyPinyinPair() {
  PinyinParser pinyin_parser;
  int8_t amount = pinyin_parser.GetPinyinUnitPartsAmount();
  for (int8_t count = 0; count < amount; ++count)
    *(*(fuzzy_pair_table_ + count) + 1) = -1;
}

/**
 * 备份用户词语.
 */
void PhraseManager::BackupUserPhrase() {
  /* 查询用户词语数据代理集合 */
  PhraseProxySite *phrase_proxy_site = NULL;
  for (std::list<PhraseProxySite *>::iterator iterator =
           phrase_proxy_site_list_.begin();
       iterator != phrase_proxy_site_list_.end();
       ++iterator) {
    if ((*iterator)->type_ == USER_TYPE) {
      phrase_proxy_site = *iterator;
      break;
    }
  }
  if (!phrase_proxy_site)
    return;

  /* 写出内存数据 */
  UserPhrase *user_phrase = (UserPhrase *)phrase_proxy_site->phrase_;
  user_phrase->WritePhraseTree();

  /* 更新用户词语文件(多绕圈可避免掉电错误) */
  char *path = NULL;
  asprintf(&path, "%s~", user_path_);
  unlink(path);  // 删除可能错误的文件
  xcopy(backup_path_, path);
  rename(path, user_path_);
  free(path);
}

/**
 * 删除词语数据.
 * @param phrase_datum 词语数据
 */
void PhraseManager::DeletePhraseDatum(const PhraseDatum *phrase_datum) const {
  /* 如果本词语不是用户词语，则直接退出即可 */
  if (phrase_datum->phrase_data_offset_ < UserPhrasePoint)
    return;

  /* 查询用户词语数据代理集合 */
  PhraseProxySite *phrase_proxy_site = NULL;
  for (std::list<PhraseProxySite *>::const_iterator iterator =
           phrase_proxy_site_list_.begin();
       iterator != phrase_proxy_site_list_.end();
       ++iterator) {
    if ((*iterator)->type_ == USER_TYPE) {
      phrase_proxy_site = *iterator;
      break;
    }
  }
  if (!phrase_proxy_site)
    return;

  /* 删除词语 */
  UserPhrase *user_phrase = (UserPhrase *)phrase_proxy_site->phrase_;
  user_phrase->DeletePhraseFromTree(phrase_datum);
}

/**
 * 反馈词语数据.
 * @param phrase_datum 词语数据
 */
void PhraseManager::FeedbackPhraseDatum(const PhraseDatum *phrase_datum) const {
  /* 如果本词语是无效词语，则直接退出即可 */
  if (phrase_datum->phrase_data_offset_ == InvalidPhraseType)
    return;

  /* 查询用户词语数据代理集合 */
  PhraseProxySite *phrase_proxy_site = NULL;
  for (std::list<PhraseProxySite *>::const_iterator iterator =
           phrase_proxy_site_list_.begin();
       iterator != phrase_proxy_site_list_.end();
       ++iterator) {
    if ((*iterator)->type_ == USER_TYPE) {
      phrase_proxy_site = *iterator;
      break;
    }
  }
  if (!phrase_proxy_site)
    return;

  /* 将词语数据反馈到用户词语中 */
  UserPhrase *user_phrase = (UserPhrase *)phrase_proxy_site->phrase_;
  if (phrase_datum->phrase_data_offset_ >= UserPhrasePoint)
    user_phrase->IncreasePhraseFrequency(phrase_datum);
  else
    user_phrase->InsertPhraseToTree(phrase_datum);
}

/**
 * 查找与汉字代理数组相匹配的词语数据代理.
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理储存点链表
 */
std::list<PhraseProxyStorage *> *PhraseManager::SearchMatchablePhrase(
    const CharsProxy *chars_proxy, int chars_proxy_length) const {
  std::list<PhraseProxyStorage *> *storage_list =
      new std::list<PhraseProxyStorage *>;
  for (std::list<PhraseProxySite *>::const_iterator iterator =
           phrase_proxy_site_list_.begin();
       iterator != phrase_proxy_site_list_.end();
       ++iterator) {
    AbstractPhrase *phrase = (*iterator)->phrase_;
    std::list<PhraseProxy *> *phrase_proxy_list =
        phrase->SearchMatchablePhrase(chars_proxy, chars_proxy_length);
    if (phrase_proxy_list) {
      PhraseProxyStorage *storage = new PhraseProxyStorage;
      storage_list->push_back(storage);
      storage->phrase_proxy_site_ = *iterator;
      storage->phrase_proxy_list_ = phrase_proxy_list;
    }
  }
  if (storage_list->empty()) {
    delete storage_list;
    storage_list = NULL;
  }

  return storage_list;
}

/**
 * 查找与汉字代理数组最相匹配的词语数据代理.
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理储存点
 */
PhraseProxyStorage *PhraseManager::SearchPreferPhrase(
                                       const CharsProxy *chars_proxy,
                                       int chars_proxy_length) const {
  /* 查找最佳词语 */
  PhraseProxySite *phrase_proxy_site = NULL;
  PhraseProxy *phrase_proxy = NULL;
  for (std::list<PhraseProxySite *>::const_iterator iterator =
           phrase_proxy_site_list_.begin();
       iterator != phrase_proxy_site_list_.end();
       ++iterator) {
    PhraseProxySite *local_phrase_proxy_site = *iterator;
    AbstractPhrase *phrase = local_phrase_proxy_site->phrase_;
    PhraseProxy *local_phrase_proxy =
        phrase->SearchPreferPhrase(chars_proxy, chars_proxy_length);
    if (!local_phrase_proxy)
      continue;
    if (!phrase_proxy ||
        phrase_proxy->chars_proxy_length_ <
            local_phrase_proxy->chars_proxy_length_ ||
        (phrase_proxy->chars_proxy_length_ ==
             local_phrase_proxy->chars_proxy_length_ &&
         phrase_proxy_site->priority_ <
             local_phrase_proxy_site->priority_)) {
      delete phrase_proxy;
      phrase_proxy_site = local_phrase_proxy_site;
      phrase_proxy = local_phrase_proxy;
    } else {
      delete local_phrase_proxy;
    }
  }

  /* 构建返回值 */
  PhraseProxyStorage *phrase_proxy_storage = NULL;
  if (phrase_proxy) {
    phrase_proxy_storage = new PhraseProxyStorage;
    phrase_proxy_storage->phrase_proxy_site_ = phrase_proxy_site;
    phrase_proxy_storage->phrase_proxy_list_ = new std::list<PhraseProxy *>;
    phrase_proxy_storage->phrase_proxy_list_->push_back(phrase_proxy);
  }

  return phrase_proxy_storage;
}

/**
 * 获取拼音矫正表.
 * @return 拼音矫正表
 */
const std::list<OuterMendPinyinPair *> *PhraseManager::GetMendPinyinTable() const {
  return &mend_pair_table_;
}

/**
 * 获取实例对象.
 * @return 实例对象
 */
PhraseManager *PhraseManager::GetInstance() {
  static PhraseManager instance;
  return &instance;
}

/**
 * 类构造函数.
 */
PhraseManager::PhraseManager()
    : fuzzy_pair_table_(NULL), user_path_(NULL), backup_path_(NULL) {
  PinyinParser pinyin_parser;
  int8_t amount = pinyin_parser.GetPinyinUnitPartsAmount();
  fuzzy_pair_table_ = (int8_t **)malloc(sizeof(int8_t *) * amount);
  for (int8_t count = 0; count < amount; ++count) {
    *(fuzzy_pair_table_ + count) = (int8_t *)malloc(sizeof(int8_t) * 2);
    **(fuzzy_pair_table_ + count) = count;
    *(*(fuzzy_pair_table_ + count) + 1) = -1;
  }
}

/**
 * 类析构函数.
 */
PhraseManager::~PhraseManager() {
  /* 备份用户词语 */
  BackupUserPhrase();
  /* 释放集合链表 */
  STL_DELETE_DATA(phrase_proxy_site_list_, std::list<PhraseProxySite *>);
  /* 释放拼音矫正表 */
  STL_DELETE_DATA(mend_pair_table_, std::list<OuterMendPinyinPair *>);
  /* 释放模糊拼音对照表 */
  PinyinParser pinyin_parser;
  int8_t amount = pinyin_parser.GetPinyinUnitPartsAmount();
  for (int8_t count = 0; count < amount; ++count)
    free(*(fuzzy_pair_table_ + count));
  free(fuzzy_pair_table_);
  /* 释放码表路径 */
  free(user_path_);
  unlink(backup_path_);  // 移除备份文件
  free(backup_path_);
 }

/**
 * 分割码表文件信息串的各部分.
 * @param string 源串
 * @retval mbfile 码表文件
 * @retval priority 优先级
 * @return 串是否合法
 */
bool PhraseManager::BreakMbfileString(char *string, const char **mbfile,
                                      const char **priority) {
  char *ptr = string + strspn(string, "\x20\t\r\n");
  if (*ptr == '\0' || *ptr == '#')
    return false;
  *mbfile = ptr;

  if (*(ptr += strcspn(ptr, "\x20\t\r\n")) == '\0')
    return false;
  *ptr = '\0';
  ++ptr;
  if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0')
    return false;
  *priority = ptr;

  return true;
}

/**
 * 创建词语数据代理的集合.
 * @param mbfile 码表文件
 * @param priority 优先级
 * @param type 集合类型
 * @return 词语数据代理的集合
 */
PhraseProxySite *PhraseManager::CreatePhraseProxySite(
                                    const char *mbfile, int priority,
                                    PhraseProxySiteType type) {
  PhraseProxySite *phrase_proxy_site = new PhraseProxySite;
  switch (type) {
    case SYSTEM_TYPE:
      phrase_proxy_site->phrase_ = new SystemPhrase;
      break;
    case USER_TYPE:
      phrase_proxy_site->phrase_ = new UserPhrase;
      break;
    default:
      assert(false);
  }
  phrase_proxy_site->phrase_->BuildPhraseTree(mbfile);
  phrase_proxy_site->phrase_->SetFuzzyPinyinTable(
      (const int8_t **)fuzzy_pair_table_);
  phrase_proxy_site->priority_ = priority;
  phrase_proxy_site->type_ = type;
  return phrase_proxy_site;
}
