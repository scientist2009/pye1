//
// C++ Implementation: system_phrase
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "system_phrase.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "global.h"
#include "output.h"
#include "wrapper.h"

/**
 * 类构造函数.
 */
SystemPhrase::SystemPhrase()
    : fuzzy_pair_table_(NULL), index_offset_(0), fd_(-1) {
}

/**
 * 类析构函数.
 */
SystemPhrase::~SystemPhrase() {
  close(fd_);
}

/**
 * 构建词语树.
 * @param mbfile 系统码表文件
 */
void SystemPhrase::BuildPhraseTree(const char *mbfile) {
  /* 打开码表文件 */
  if ((fd_ = open(mbfile, O_RDONLY)) == -1) {
    pwarning("Open file \"%s\" failed, %s", mbfile, strerror(errno));
    return;
  }

  /* 读取词语树 */
  ReadPhraseTree();
}

/**
 * 设置模糊拼音单元部件对照表.
 * @param fuzzy_pair_table 对照表
 * @note 在查询词语之前必须调用本函数.
 */
void SystemPhrase::SetFuzzyPinyinTable(const int8_t **fuzzy_pair_table) {
  fuzzy_pair_table_ = fuzzy_pair_table;
}

/**
 * 查找与汉字代理数组相匹配的词语数据代理.
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理链表
 */
std::list<PhraseProxy *> *SystemPhrase::SearchMatchablePhrase(
                                            const CharsProxy *chars_proxy,
                                            int chars_proxy_length) {
  /* 查询词语 */
  std::list<std::list<PhraseProxy *> *> multi_phrase_list;
  for (const int8_t *index_ptr =
           *(fuzzy_pair_table_ + chars_proxy->major_index_);
       *index_ptr != -1;
       ++index_ptr) {
    std::list<PhraseProxy *> *phrase_list =
        SearchMatchablePhrase(*index_ptr, chars_proxy, chars_proxy_length);
    if (phrase_list)
      multi_phrase_list.push_back(phrase_list);
  }
  if (multi_phrase_list.empty())
    return NULL;

  /* 合并词语 */
  std::list<PhraseProxy *> *phrase_list = new std::list<PhraseProxy *>;
  std::list<std::list<PhraseProxy *> *>::iterator previous_iterator;
  do {
    /* 查找优先级最高的词语 */
    std::list<std::list<PhraseProxy *> *>::iterator selected_iterator;
    PhraseProxy *selected_phrase_proxy = NULL;
    bool pass(false), priority(false);
    for (std::list<std::list<PhraseProxy *> *>::iterator iterator =
             multi_phrase_list.begin();
         iterator != multi_phrase_list.end();
         ++iterator) {
      PhraseProxy *local_phrase_proxy = (*iterator)->front();
      if (!selected_phrase_proxy ||
          selected_phrase_proxy->chars_proxy_length_ <
              local_phrase_proxy->chars_proxy_length_ ||
          (selected_phrase_proxy->chars_proxy_length_ ==
              local_phrase_proxy->chars_proxy_length_ &&
           priority)) {
        selected_iterator = iterator;
        selected_phrase_proxy = local_phrase_proxy;
        priority = false;
      }
      if (!pass)
        pass = priority = previous_iterator == iterator;
    }
    /* 加入词语 */
    phrase_list->push_back(selected_phrase_proxy);
    (*selected_iterator)->pop_front();
    if ((*selected_iterator)->empty()) {
      delete *selected_iterator;
      selected_iterator = multi_phrase_list.erase(selected_iterator);
      --selected_iterator;
    }
    /* 记录本轮被选中的迭代器 */
    previous_iterator = selected_iterator;
  } while (!multi_phrase_list.empty());

  return phrase_list;
}

/**
 * 查找与汉字代理数组最相匹配的词语数据代理.
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理
 */
PhraseProxy *SystemPhrase::SearchPreferPhrase(const CharsProxy *chars_proxy,
                                              int chars_proxy_length) {
  /* 查询词语 */
  std::list<PhraseProxy *> phrase_list;
  for (const int8_t *index_ptr =
           *(fuzzy_pair_table_ + chars_proxy->major_index_);
       *index_ptr != -1;
       ++index_ptr) {
    PhraseProxy *phrase_proxy =
        SearchPreferPhrase(*index_ptr, chars_proxy, chars_proxy_length);
    if (phrase_proxy)
      phrase_list.push_back(phrase_proxy);
  }
  if (phrase_list.empty())
    return NULL;

  /* 查找优先级最高的词语 */
  std::list<PhraseProxy *>::iterator selected_iterator;
  PhraseProxy *selected_phrase_proxy = NULL;
  for (std::list<PhraseProxy *>::iterator iterator = phrase_list.begin();
       iterator != phrase_list.end();
       ++iterator) {
    PhraseProxy *local_phrase_proxy = *iterator;
    if (!selected_phrase_proxy ||
        selected_phrase_proxy->chars_proxy_length_ <
            local_phrase_proxy->chars_proxy_length_) {
      selected_iterator = iterator;
      selected_phrase_proxy = local_phrase_proxy;
    }
  }
  /* 删除词语 */
  phrase_list.erase(selected_iterator);
  DELETE_LIST_DATA(phrase_list, PhraseProxy);

  return selected_phrase_proxy;
}

/**
 * 解析词语数据代理所表示的词语数据.
 * @param phrase_proxy 词语数据代理
 * @return 词语数据
 */
PhraseDatum *SystemPhrase::AnalyzePhraseProxy(const PhraseProxy *phrase_proxy) {
  PhraseDatum *phrase_datum = new PhraseDatum;
  phrase_datum->chars_proxy_ =
      new CharsProxy[phrase_proxy->chars_proxy_length_];
  memcpy(phrase_datum->chars_proxy_, phrase_proxy->chars_proxy_,
         sizeof(CharsProxy) * phrase_proxy->chars_proxy_length_);
  phrase_datum->chars_proxy_length_ = phrase_proxy->chars_proxy_length_;
  lseek(fd_, phrase_proxy->phrase_data_offset_, SEEK_SET);
  xread(fd_, &phrase_datum->phrase_data_offset_,
        sizeof(phrase_datum->phrase_data_offset_));
  lseek(fd_, phrase_datum->phrase_data_offset_, SEEK_SET);
  xread(fd_, &phrase_datum->raw_data_length_,
        sizeof(phrase_datum->raw_data_length_));
  phrase_datum->raw_data_ = malloc(phrase_datum->raw_data_length_);
  xread(fd_, phrase_datum->raw_data_, phrase_datum->raw_data_length_);
  phrase_datum->phrase_data_offset_ = SystemPhraseType;
  return phrase_datum;
}

/**
 * 读取系统码表文件的索引部分，并构建词语树.
 */
void SystemPhrase::ReadPhraseTree() {
  int offset = 0;  // 相对偏移量

  /* 构建根节点 */
  SystemPhraseRootNode *root_node = &root_;
  xread(fd_, &root_node->max_index_, sizeof(root_node->max_index_));
  root_node->table_ = new SystemPhraseIndexNode[root_node->max_index_ + 1];
  int8_t index = -1;  // 当前索引值
  do {
    xread(fd_, &index, sizeof(index));
    /* 构建索引节点 */
    SystemPhraseIndexNode *index_node = root_node->table_ + index;
    xread(fd_, &index_node->max_length_, sizeof(index_node->max_length_));
    index_node->table_ = new SystemPhraseLengthNode[index_node->max_length_];
    int length = 0;  // 当前长度
    do {
      xread(fd_, &length, sizeof(length));
      /* 构建长度节点 */
      SystemPhraseLengthNode *length_node = index_node->table_ + length - 1;
      xread(fd_, &length_node->phrase_amount_,
            sizeof(length_node->phrase_amount_));
      size_t number = length * length_node->phrase_amount_;
      length_node->chars_proxy_ = new CharsProxy[number];
      xread(fd_, length_node->chars_proxy_, sizeof(CharsProxy) * number);
      length_node->index_offset_ = offset;
      offset += sizeof(int) * length_node->phrase_amount_;
    } while (length < index_node->max_length_);
  } while (index < root_node->max_index_);

  /* 获取绝对偏移量 */
  index_offset_ = lseek(fd_, 0, SEEK_CUR);
}

/**
 * 查找位于本索引值下与汉字代理数组相匹配的词语数据代理.
 * @param chars_proxy_index 索引值
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理链表
 */
std::list<PhraseProxy *> *SystemPhrase::SearchMatchablePhrase(
                                            int8_t chars_proxy_index,
                                            const CharsProxy *chars_proxy,
                                            int chars_proxy_length) {
  /* 检查条件是否满足 */
  if (root_.max_index_ < chars_proxy_index)
    return NULL;
  SystemPhraseIndexNode *index_node = root_.table_ + chars_proxy_index;
  if (index_node->max_length_ == 0)
    return NULL;

  /* 查询数据 */
  std::list<PhraseProxy *> *phrase_list = new std::list<PhraseProxy *>;
  int length = chars_proxy_length <= index_node->max_length_ ?
                   chars_proxy_length : index_node->max_length_;
  for (; length >= 1; --length) {
    SystemPhraseLengthNode *length_node = index_node->table_ + length - 1;
    uint number = length_node->phrase_amount_;
    while (number >= 1) {
      --number;
      if (CharsProxyCmp(fuzzy_pair_table_,
                        chars_proxy,
                        length_node->chars_proxy_ + length * number,
                        length)) {
        PhraseProxy *phrase_proxy = new PhraseProxy;
        phrase_list->push_back(phrase_proxy);
        phrase_proxy->chars_proxy_ =
            length_node->chars_proxy_ + length * number;
        phrase_proxy->chars_proxy_length_ = length;
        phrase_proxy->phrase_data_offset_ =
            index_offset_ + length_node->index_offset_ + sizeof(int) * number;
      }
    }
  }

  /* 检查返回值 */
  if (phrase_list->empty()) {
    delete phrase_list;
    phrase_list = NULL;
  }

  return phrase_list;
}

/**
 * 查找位于本索引值下与汉字代理数组最相匹配的词语数据代理.
 * @param chars_proxy_index 索引值
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组有效长度
 * @return 词语数据代理
 */
PhraseProxy *SystemPhrase::SearchPreferPhrase(int8_t chars_proxy_index,
                                              const CharsProxy *chars_proxy,
                                              int chars_proxy_length) {
  /* 检查条件是否满足 */
  if (root_.max_index_ < chars_proxy_index)
    return NULL;
  SystemPhraseIndexNode *index_node = root_.table_ + chars_proxy_index;
  if (index_node->max_length_ == 0)
    return NULL;

  /* 查询数据 */
  PhraseProxy *phrase_proxy = NULL;
  int length = chars_proxy_length <= index_node->max_length_ ?
                   chars_proxy_length : index_node->max_length_;
  for (; length >= 1; --length) {
    SystemPhraseLengthNode *length_node = index_node->table_ + length - 1;
    uint number = length_node->phrase_amount_;
    while (number >= 1) {
      --number;
      if (CharsProxyCmp(fuzzy_pair_table_,
                        chars_proxy,
                        length_node->chars_proxy_ + length * number,
                        length)) {
        phrase_proxy = new PhraseProxy;
        phrase_proxy->chars_proxy_ =
            length_node->chars_proxy_ + length * number;
        phrase_proxy->chars_proxy_length_ = length;
        phrase_proxy->phrase_data_offset_ =
            index_offset_ + length_node->index_offset_ + sizeof(int) * number;
        break;
      }
    }
    if (phrase_proxy)
      break;
  }

  return phrase_proxy;
}
