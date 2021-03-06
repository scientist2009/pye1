//
// C++ Implementation: umb_creater
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "umb_creater.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "engine/pye_output.h"
#include "engine/pye_wrapper.h"

/**
 * 类构造函数.
 */
UMBCreater::UMBCreater() {
}

/**
 * 类析构函数.
 */
UMBCreater::~UMBCreater() {
}

/**
 * 根据数据文件创建词语树.
 * @param data_file 数据文件
 */
void UMBCreater::BuildPhraseTree(const char *data_file) {
  /* 打开数据文件 */
  FILE *stream = fopen(data_file, "r");
  if (!stream)
    errx(1, "Fopen file \"%s\" failed, %s", data_file, strerror(errno));

  /* 读取文件数据，分析，插入到索引树 */
  uint phrases(0), converts(0);
  char *lineptr = NULL;
  size_t n = 0;
  while (getline(&lineptr, &n, stream) != -1) {
    pmessage("\rReading Phrase: %u", ++phrases);
    const char *phrase(NULL), *pinyin(NULL), *frequency(NULL);
    if (!BreakPhraseString(lineptr, &phrase, &pinyin, &frequency))
      continue;
    PhraseDatum *datum = CreatePhraseDatum(phrase, pinyin, frequency);
    if (!datum)
      continue;
    InsertDatumToTree(datum);
    ++converts;
  }
  pmessage("\n%u Phrases, %u Converted!\n", phrases, converts);
  free(lineptr);

  /* 关闭数据文件 */
  fclose(stream);
}

/**
 * 写出词语树，即生成码表文件.
 * @param mb_file 码表文件
 */
void UMBCreater::WritePhraseTree(const char *mb_file) {
  /* 创建码表文件 */
  int fd = open(mb_file, O_WRONLY | O_CREAT | O_EXCL, 00644);
  if (fd == -1)
    errx(1, "Open file \"%s\" failed, %s", mb_file, strerror(errno));

  /* 写出码表文件 */
  int offset = RebuildPhraseTree(fd);
  WritePhraseTree(fd, offset);
  pmessage("Finished!\n");

  /* 关闭码表文件 */
  close(fd);
}

/**
 * 分割词语数据串.
 * @param string 数据串
 * @param phrase 词语串
 * @param pinyin 拼音串
 * @param frequency 频率串
 * @return 串是否合法
 */
bool UMBCreater::BreakPhraseString(char *string, const char **phrase,
                                  const char **pinyin,
                                  const char **frequency) {
  char *ptr = string + strspn(string, "\x20\t\r\n");
  if (*ptr == '\0')
    return false;
  *phrase = ptr;

  if (*(ptr += strcspn(ptr, "\x20\t\r\n")) == '\0')
    return false;
  *ptr = '\0';
  ++ptr;
  if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0')
    return false;
  *pinyin = ptr;

  if (*(ptr += strcspn(ptr, "\x20\t\r\n")) == '\0')
    return false;
  *ptr = '\0';
  ++ptr;
  if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0')
    return false;
  *frequency = ptr;

  return true;
}

/**
 * 创建词语数据资料.
 * @param phrase 词语串
 * @param pinyin 拼音串
 * @param frequency 频率串
 * @return 词语数据资料
 */
PhraseDatum *UMBCreater::CreatePhraseDatum(const char *phrase,
                                          const char *pinyin,
                                          const char *frequency) {
  PhraseDatum *datum = new PhraseDatum;
  PinyinParser pinyin_parser;
  pinyin_parser.ParsePinyin(pinyin, &datum->chars_proxy_,
                            &datum->chars_proxy_length_);
  if (datum->chars_proxy_length_ == 0) {
    delete datum;
    return NULL;
  }

  datum->raw_data_length_ = strlen(phrase);
  datum->raw_data_ = malloc(datum->raw_data_length_);
  memcpy(datum->raw_data_, phrase, datum->raw_data_length_);
  datum->frequency_ = atoi(frequency);
  return datum;
}

/**
 * 插入词语数据资料到词语树.
 * @param datum 词语数据资料
 */
void UMBCreater::InsertDatumToTree(PhraseDatum *datum) {
  std::list<PhraseLengthNode *> *length_list =
    SearchChildByIndex(&root_.data_, datum->chars_proxy_->major_index_);
  std::list<PhraseDatum *> *datum_list =
    SearchChildByLength(length_list, datum->chars_proxy_length_);

  std::list<PhraseDatum *>::iterator iterator = datum_list->begin();
  for (; iterator != datum_list->end(); ++iterator) {
    if ((*iterator)->frequency_ >= datum->frequency_)
      break;
  }
  datum_list->insert(iterator, datum);
}

/**
 * 按汉字代理数组的索引值搜索孩子.
 * @param data_list 数据链表
 * @param index 索引值
 * @return 孩子链表
 */
std::list<PhraseLengthNode *> *UMBCreater::SearchChildByIndex(
    std::list<PhraseIndexNode *> *data_list, int index) {
  /* 确定孩子节点的位置 */
  std::list<PhraseIndexNode *>::iterator iterator = data_list->begin();
  for (; iterator != data_list->end(); ++iterator) {
    if ((*iterator)->chars_proxy_index_ >= index)
      break;
  }

  /* 获取孩子链表 */
  std::list<PhraseLengthNode *> *length_list = NULL;
  if (iterator == data_list->end() || (*iterator)->chars_proxy_index_ > index) {
    PhraseIndexNode *node = new PhraseIndexNode;
    node->chars_proxy_index_ = index;
    data_list->insert(iterator, node);
    length_list = &node->data_;
  } else {
    length_list = &(*iterator)->data_;
  }

  return length_list;
}

/**
 * 按汉字代理数组的长度搜索孩子.
 * @param data_list 数据链表
 * @param length 长度
 * @return 孩子链表
 */
std::list<PhraseDatum *> *UMBCreater::SearchChildByLength(
    std::list<PhraseLengthNode *> *data_list, int length) {
  /* 确定孩子节点的位置 */
  std::list<PhraseLengthNode *>::iterator iterator = data_list->begin();
  for (; iterator != data_list->end(); ++iterator) {
    if ((*iterator)->chars_proxy_length_ >= length)
      break;
  }

  /* 获取孩子链表 */
  std::list<PhraseDatum *> *datum_list = NULL;
  if (iterator == data_list->end() ||
      (*iterator)->chars_proxy_length_ > length) {
    PhraseLengthNode *node = new PhraseLengthNode;
    node->chars_proxy_length_ = length;
    data_list->insert(iterator, node);
    datum_list = &node->data_;
  } else {
    datum_list = &(*iterator)->data_;
  }

  return datum_list;
}

/**
 * 重构词语树，并写出数据部分.
 * @param fd 文件描述字
 * @return 索引部分的偏移量
 */
int UMBCreater::RebuildPhraseTree(int fd) {
  /* 索引偏移量占位 */
  int offset = -1;
  lseek(fd, sizeof(offset), SEEK_SET);

  /* 构建根节点 */
  std::list<PhraseIndexNode *> *index_list = &root_.data_;
  user_root_.max_index_ = index_list->back()->chars_proxy_index_;
  user_root_.table_ = new UserPhraseIndexNode[user_root_.max_index_ + 1];
  for (std::list<PhraseIndexNode *>::iterator iterator = index_list->begin();
       iterator != index_list->end();
       ++iterator) {
    int8_t chars_proxy_index = (*iterator)->chars_proxy_index_;
    /* 构建索引节点 */
    UserPhraseIndexNode *index_node = user_root_.table_ + chars_proxy_index;
    std::list<PhraseLengthNode *> *length_list = &(*iterator)->data_;
    index_node->max_length_ = length_list->back()->chars_proxy_length_;
    index_node->table_ = new UserPhraseLengthNode[index_node->max_length_];
    for (std::list<PhraseLengthNode *>::iterator iterator = length_list->begin();
         iterator != length_list->end();
         ++iterator) {
      int chars_proxy_length = (*iterator)->chars_proxy_length_;
      /* 构建长度节点 */
      UserPhraseLengthNode *length_node =
          index_node->table_ + chars_proxy_length - 1;
      std::list<PhraseDatum *> *datum_list = &(*iterator)->data_;
      length_node->phrase_amount_ = datum_list->size();
      length_node->chars_proxy_ =
          new CharsProxy[chars_proxy_length * length_node->phrase_amount_];
      length_node->phrase_attribute_ =
          new UserPhraseAttribute[length_node->phrase_amount_];
      uint number = 0;  // 初始化编号
      for (std::list<PhraseDatum *>::iterator iterator = datum_list->begin();
           iterator != datum_list->end();
           ++iterator) {
        PhraseDatum *datum = *iterator;
        /* 填充节点数据 */
        memcpy(length_node->chars_proxy_ + chars_proxy_length * number,
               datum->chars_proxy_,
               sizeof(CharsProxy) * chars_proxy_length);
        UserPhraseAttribute *phrase_attribute =
            length_node->phrase_attribute_ + number;
        phrase_attribute->datum_offset_ = lseek(fd, 0, SEEK_CUR);
        phrase_attribute->frequency_ = datum->frequency_;
        /* 写出词语数据 */
        xwrite(fd, &datum->raw_data_length_, sizeof(datum->raw_data_length_));
        xwrite(fd, datum->raw_data_, datum->raw_data_length_);
        ++number;  // 增长编号
      }
    }
  }

  offset = lseek(fd, 0, SEEK_CUR);
  return offset;
}

/**
 * 写出词语树，即索引部分.
 * @param fd 文件描述字
 * @param offset 索引部分的偏移量
 */
void UMBCreater::WritePhraseTree(int fd, int offset) {
  /* 写出索引偏移量 */
  lseek(fd, 0, SEEK_SET);
  xwrite(fd, &offset, sizeof(offset));
  /* 写出根节点的数据 */
  lseek(fd, offset, SEEK_SET);
  xwrite(fd, &user_root_, sizeof(UserPhraseRootNode));
  /* 写出索引节点的数据 */
  if (user_root_.max_index_ == -1)
    return;
  xwrite(fd, user_root_.table_,
         sizeof(UserPhraseIndexNode) * (user_root_.max_index_ + 1));
  /* 写出长度节点的数据 */
  for (int8_t index = 0; index <= user_root_.max_index_; ++index) {
    UserPhraseIndexNode *index_node = user_root_.table_ + index;
    if (index_node->max_length_ == 0)
      continue;
    xwrite(fd, index_node->table_,
           sizeof(UserPhraseLengthNode) * index_node->max_length_);
    /* 写出词语数据资料 */
    for (int length = 1; length <= index_node->max_length_; ++length) {
      UserPhraseLengthNode *length_node = index_node->table_ + length - 1;
      if (length_node->phrase_amount_ == 0)
        continue;
      xwrite(fd, length_node->chars_proxy_,
             sizeof(CharsProxy) * length * length_node->phrase_amount_);
      xwrite(fd, length_node->phrase_attribute_,
             sizeof(UserPhraseAttribute) * length_node->phrase_amount_);
    }
  }
}
