//
// C++ Implementation: user_phrase
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "user_phrase.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "pye_global.h"
#include "pye_output.h"
#include "pye_wrapper.h"

/**
 * 类构造函数.
 */
UserPhrase::UserPhrase() : fuzzy_pair_table_(NULL), index_offset_(0), fd_(-1) {
}

/**
 * 类析构函数.
 */
UserPhrase::~UserPhrase() {
  close(fd_);
}

/**
 * 构建词语树.
 * @param mbfile 用户码表文件
 */
void UserPhrase::BuildPhraseTree(const char *mbfile) {
  /* 打开码表文件 */
  if (access(mbfile, F_OK) == 0) {
    if ((fd_ = open(mbfile, O_RDWR)) == -1)
      errx(1, "Open file \"%s\" failed, %s", mbfile, strerror(errno));
  } else {
    if ((fd_ = open(mbfile, O_RDWR | O_CREAT, 00644)) == -1)
      errx(1, "Open file \"%s\" failed, %s", mbfile, strerror(errno));
    WriteEmptyPhraseTree();
  }

  /* 读取词语树 */
  ReadPhraseTree();
}

/**
 * 设置模糊拼音单元部件对照表.
 * @param fuzzy_pair_table 对照表
 * @note 在查询词语之前必须调用本函数.
 */
void UserPhrase::SetFuzzyPinyinTable(const int8_t **fuzzy_pair_table) {
  fuzzy_pair_table_ = fuzzy_pair_table;
}

/**
 * 查找与汉字代理数组相匹配的词语数据代理.
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理链表
 */
std::list<PhraseProxy *> *UserPhrase::SearchMatchablePhrase(
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
           (selected_phrase_proxy->frequency_ <
                local_phrase_proxy->frequency_ ||
            (selected_phrase_proxy->frequency_ ==
                 local_phrase_proxy->frequency_ &&
             priority)))) {
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
PhraseProxy *UserPhrase::SearchPreferPhrase(const CharsProxy *chars_proxy,
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
            local_phrase_proxy->chars_proxy_length_ ||
        (selected_phrase_proxy->chars_proxy_length_ ==
             local_phrase_proxy->chars_proxy_length_ &&
         selected_phrase_proxy->frequency_ <
             local_phrase_proxy->frequency_)) {
      selected_iterator = iterator;
      selected_phrase_proxy = local_phrase_proxy;
    }
  }
  /* 删除词语 */
  phrase_list.erase(selected_iterator);
  STL_DELETE_DATA(phrase_list, std::list<PhraseProxy *>);

  return selected_phrase_proxy;
}

/**
 * 解析词语数据代理所表示的词语数据.
 * @param phrase_proxy 词语数据代理
 * @return 词语数据
 */
PhraseDatum *UserPhrase::AnalyzePhraseProxy(const PhraseProxy *phrase_proxy) {
  PhraseDatum *phrase_datum = new PhraseDatum;
  phrase_datum->chars_proxy_ =
      new CharsProxy[phrase_proxy->chars_proxy_length_];
  memcpy(phrase_datum->chars_proxy_, phrase_proxy->chars_proxy_,
         sizeof(CharsProxy) * phrase_proxy->chars_proxy_length_);
  phrase_datum->chars_proxy_length_ = phrase_proxy->chars_proxy_length_;
  phrase_datum->phrase_data_offset_ = phrase_proxy->phrase_data_offset_;
  lseek(fd_, phrase_datum->phrase_data_offset_, SEEK_SET);
  xread(fd_, &phrase_datum->raw_data_length_,
        sizeof(phrase_datum->raw_data_length_));
  phrase_datum->raw_data_ = malloc(phrase_datum->raw_data_length_);
  xread(fd_, phrase_datum->raw_data_, phrase_datum->raw_data_length_);

  return phrase_datum;
}

/**
 * 插入词语数据资料到词语树.
 * @param phrase_datum 词语数据资料
 */
void UserPhrase::InsertPhraseToTree(const PhraseDatum *phrase_datum) {
  /* 定位词语的汉字代理数组的索引节点 */
  int8_t index = phrase_datum->chars_proxy_->major_index_;
  if (root_.max_index_ < index) {
    UserPhraseIndexNode *table = root_.table_;
    root_.table_ = new UserPhraseIndexNode[index + 1];
    if (table) {
      size_t size = sizeof(UserPhraseIndexNode) * (root_.max_index_ + 1);
      memcpy(root_.table_, table, size);
      bzero(table, size);
      delete [] table;
    }
    root_.max_index_ = index;
  }
  UserPhraseIndexNode *index_node = root_.table_ + index;

  /* 定位词语的汉字代理数组的长度节点 */
  int length = phrase_datum->chars_proxy_length_;
  if (index_node->max_length_ < length) {
    UserPhraseLengthNode *table = index_node->table_;
    index_node->table_ = new UserPhraseLengthNode[length];
    if (table) {
      size_t size = sizeof(UserPhraseLengthNode) * index_node->max_length_;
      memcpy(index_node->table_, table, size);
      bzero(table, size);
      delete [] table;
    }
    index_node->max_length_ = length;
  }
  UserPhraseLengthNode *length_node = index_node->table_ + length - 1;

  /* 定位词语位置 */
  uint number = 0;
  uint amount = length_node->phrase_amount_;
  for (; number < amount; ++number) {
    if ((length_node->phrase_attribute_ + number)->frequency_ > 1)
      break;
  }
  /* 插入词语的汉字代理数组 */
  CharsProxy *chars_proxy = length_node->chars_proxy_;
  length_node->chars_proxy_ = new CharsProxy[length * (amount + 1)];
  if (number != 0)
    memcpy(length_node->chars_proxy_,
           chars_proxy,
           sizeof(CharsProxy) * length * number);
  memcpy(length_node->chars_proxy_ + length * number,
         phrase_datum->chars_proxy_,
         sizeof(CharsProxy) * length);
  if (number != amount)
    memcpy(length_node->chars_proxy_ + length * (number + 1),
           chars_proxy + length * number,
           sizeof(CharsProxy) * length * (amount - number));
  delete [] chars_proxy;
  /* 插入词语的属性 */
  UserPhraseAttribute *phrase_attribute = length_node->phrase_attribute_;
  length_node->phrase_attribute_ = new UserPhraseAttribute[amount + 1];
  if (number != 0)
    memcpy(length_node->phrase_attribute_,
           phrase_attribute,
           sizeof(UserPhraseAttribute) * number);
  UserPhraseAttribute *attribute = length_node->phrase_attribute_ + number;
  attribute->datum_offset_ = index_offset_;
  attribute->frequency_ = 1;
  if (number != amount)
    memcpy(length_node->phrase_attribute_ + number + 1,
           phrase_attribute + number,
           sizeof(UserPhraseAttribute) * (amount - number));
  delete [] phrase_attribute;
  /* 写出词语数据 */
  lseek(fd_, index_offset_, SEEK_SET);
  xwrite(fd_, &phrase_datum->raw_data_length_,
         sizeof(phrase_datum->raw_data_length_));
  xwrite(fd_, phrase_datum->raw_data_, phrase_datum->raw_data_length_);
  /* 更新数据 */
  ++(length_node->phrase_amount_);
  index_offset_ += sizeof(phrase_datum->raw_data_length_) +
                   phrase_datum->raw_data_length_;
}

/**
 * 从词语树删除指定词语.
 * @param phrase_datum 词语数据资料
 */
void UserPhrase::DeletePhraseFromTree(const PhraseDatum *phrase_datum) {
  /* 检查条件是否满足 */
  int8_t index = phrase_datum->chars_proxy_->major_index_;
  if (root_.max_index_ < index)
    return;
  UserPhraseIndexNode *index_node = root_.table_ + index;
  int length = phrase_datum->chars_proxy_length_;
  if (index_node->max_length_ < length)
    return;
  UserPhraseLengthNode *length_node = index_node->table_ + length - 1;

  /* 定位词语位置 */
  uint number = 0;
  uint amount = length_node->phrase_amount_;
  int offset = phrase_datum->phrase_data_offset_;
  for (; number < amount; ++number) {
    if ((length_node->phrase_attribute_ + number)->datum_offset_ == offset)
      break;
  }
  if (number == amount)
    return;
  /* 移动数据 */
  if (number != amount - 1) {
    memmove(length_node->chars_proxy_ + length * number,
            length_node->chars_proxy_ + length * (number + 1),
            sizeof(CharsProxy) * length * (amount - number - 1));
    memmove(length_node->phrase_attribute_ + number,
            length_node->phrase_attribute_ + number + 1,
            sizeof(UserPhraseAttribute) * (amount - number - 1));
  }
  /* 更新数据 */
  --(length_node->phrase_amount_);
  if (length_node->phrase_amount_ == 0) {
    delete [] length_node->chars_proxy_;
    length_node->chars_proxy_ = NULL;
    delete [] length_node->phrase_attribute_;
    length_node->phrase_attribute_ = NULL;
  }
}

/**
 * 增加指定词语的使用频率.
 * @param phrase_datum 词语数据
 */
void UserPhrase::IncreasePhraseFrequency(const PhraseDatum *phrase_datum) {
  /* 检查条件是否满足 */
  int8_t index = phrase_datum->chars_proxy_->major_index_;
  if (root_.max_index_ < index)
    return;
  UserPhraseIndexNode *index_node = root_.table_ + index;
  int length = phrase_datum->chars_proxy_length_;
  if (index_node->max_length_ < length)
    return;
  UserPhraseLengthNode *length_node = index_node->table_ + length - 1;

  /* 定位词语位置 */
  uint number = 0;
  uint amount = length_node->phrase_amount_;
  int offset = phrase_datum->phrase_data_offset_;
  for (; number < amount; ++number) {
    if ((length_node->phrase_attribute_ + number)->datum_offset_ == offset)
      break;
  }
  if (number == amount)
    return;
  ++((length_node->phrase_attribute_ + number)->frequency_);
  /* 查询新位置 */
  uint position = number + 1;
  int frequency = (length_node->phrase_attribute_ + number)->frequency_;
  for (; position < amount; ++position) {
    if ((length_node->phrase_attribute_ + position)->frequency_ > frequency)
      break;
  }
  if (number + 1 == position)
    return;
  /* 移动数据 */
  memmove(length_node->chars_proxy_ + length * number,
          length_node->chars_proxy_ + length * (number + 1),
          sizeof(CharsProxy) * length * (position - number - 1));
  memcpy(length_node->chars_proxy_ + length * (position - 1),
         phrase_datum->chars_proxy_,
         sizeof(CharsProxy) * length);
  UserPhraseAttribute attribute = *(length_node->phrase_attribute_ + number);
  memmove(length_node->phrase_attribute_ + number,
          length_node->phrase_attribute_ + number + 1,
          sizeof(UserPhraseAttribute) * (position - number - 1));
  memmove(length_node->phrase_attribute_ + position - 1,
          &attribute,
          sizeof(attribute));
}

/**
 * 写出词语树，即索引部分.
 */
void UserPhrase::WritePhraseTree() {
  /* 写出索引偏移量 */
  lseek(fd_, 0, SEEK_SET);
  xwrite(fd_, &index_offset_, sizeof(index_offset_));
  /* 写出根节点的数据 */
  lseek(fd_, index_offset_, SEEK_SET);
  xwrite(fd_, &root_, sizeof(UserPhraseRootNode));
  /* 写出索引节点的数据 */
  if (root_.max_index_ == -1)
    return;
  xwrite(fd_, root_.table_,
         sizeof(UserPhraseIndexNode) * (root_.max_index_ + 1));
  /* 写出长度节点的数据 */
  for (int8_t count = 0; count <= root_.max_index_; ++count) {
    UserPhraseIndexNode *index_node = root_.table_ + count;
    if (index_node->max_length_ == 0)
      continue;
    xwrite(fd_, index_node->table_,
           sizeof(UserPhraseLengthNode) * index_node->max_length_);
    /* 写出词语数据资料 */
    for (int length = 1; length <= index_node->max_length_; ++length) {
      UserPhraseLengthNode *length_node = index_node->table_ + length - 1;
      if (length_node->phrase_amount_ == 0)
        continue;
      xwrite(fd_, length_node->chars_proxy_,
             sizeof(CharsProxy) * length * length_node->phrase_amount_);
      xwrite(fd_, length_node->phrase_attribute_,
             sizeof(UserPhraseAttribute) * length_node->phrase_amount_);
    }
  }
}

/**
 * 写出空树.
 */
void UserPhrase::WriteEmptyPhraseTree() {
  int offset = sizeof(int);
  UserPhraseRootNode root;

  lseek(fd_, 0, SEEK_SET);
  xwrite(fd_, &offset, sizeof(offset));
  xwrite(fd_, &root, sizeof(root));
}

/**
 * 读取用户码表文件的索引部分，并构建词语树.
 */
void UserPhrase::ReadPhraseTree() {
  /* 读取索引偏移量 */
  lseek(fd_, 0, SEEK_SET);
  xread(fd_, &index_offset_, sizeof(index_offset_));
  /* 读取根节点的数据 */
  lseek(fd_, index_offset_, SEEK_SET);
  xread(fd_, &root_, sizeof(UserPhraseRootNode));
  /* 读取索引节点的数据 */
  if (root_.max_index_ == -1)
    return;
  root_.table_ = new UserPhraseIndexNode[root_.max_index_ + 1];
  xread(fd_, root_.table_,
         sizeof(UserPhraseIndexNode) * (root_.max_index_ + 1));
  /* 读取长度节点的数据 */
  for (int8_t count = 0; count <= root_.max_index_; ++count) {
    UserPhraseIndexNode *index_node = root_.table_ + count;
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
 * 查找位于本索引值下与汉字代理数组相匹配的词语数据代理.
 * @param chars_proxy_index 索引值
 * @param chars_proxy 汉字代理数组
 * @param chars_proxy_length 汉字代理数组的有效长度
 * @return 词语数据代理链表
 */
std::list<PhraseProxy *> *UserPhrase::SearchMatchablePhrase(
                                          int8_t chars_proxy_index,
                                          const CharsProxy *chars_proxy,
                                          int chars_proxy_length) {
  /* 检查条件是否满足 */
  if (root_.max_index_ < chars_proxy_index)
    return NULL;
  UserPhraseIndexNode *index_node = root_.table_ + chars_proxy_index;
  if (index_node->max_length_ == 0)
    return NULL;

  /* 查询数据 */
  std::list<PhraseProxy *> *phrase_list = new std::list<PhraseProxy *>;
  int length = chars_proxy_length <= index_node->max_length_ ?
                   chars_proxy_length : index_node->max_length_;
  for (; length >= 1; --length) {
    UserPhraseLengthNode *length_node = index_node->table_ + length - 1;
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
        UserPhraseAttribute *attribute =
            length_node->phrase_attribute_ + number;
        phrase_proxy->phrase_data_offset_ = attribute->datum_offset_;
        phrase_proxy->frequency_ = attribute->frequency_;
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
PhraseProxy *UserPhrase::SearchPreferPhrase(int8_t chars_proxy_index,
                                            const CharsProxy *chars_proxy,
                                            int chars_proxy_length) {
  /* 检查条件是否满足 */
  if (root_.max_index_ < chars_proxy_index)
    return NULL;
  UserPhraseIndexNode *index_node = root_.table_ + chars_proxy_index;
  if (index_node->max_length_ == 0)
    return NULL;

  /* 查询数据 */
  PhraseProxy *phrase_proxy = NULL;
  int length = chars_proxy_length <= index_node->max_length_ ?
                   chars_proxy_length : index_node->max_length_;
  for (; length >= 1; --length) {
    UserPhraseLengthNode *length_node = index_node->table_ + length - 1;
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
        UserPhraseAttribute *attribute =
            length_node->phrase_attribute_ + number;
        phrase_proxy->phrase_data_offset_ = attribute->datum_offset_;
        phrase_proxy->frequency_ = attribute->frequency_;
        break;
      }
    }
    if (phrase_proxy)
      break;
  }

  return phrase_proxy;
}
