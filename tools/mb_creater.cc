//
// C++ Implementation: mb_creater
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mb_creater.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "engine/output.h"
#include "engine/wrapper.h"

/**
 * 类构造函数.
 */
MBCreater::MBCreater() {
}

/**
 * 类析构函数.
 */
MBCreater::~MBCreater() {
}

/**
 * 根据数据文件创建词语树.
 * @param data_file 数据文件
 */
void MBCreater::BuildPhraseTree(const char *data_file) {
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
void MBCreater::WritePhraseTree(const char *mb_file) {
  /* 创建码表文件 */
  int fd = open(mb_file, O_WRONLY | O_CREAT | O_EXCL, 00644);
  if (fd == -1)
    errx(1, "Open file \"%s\" failed, %s", mb_file, strerror(errno));

  /* 写出纯索引&数据索引&词语数据 */
  pmessage("Writing pure index part ...\n");
  int offset = 0;
  uint amount = WritePureIndexPart(fd, &offset);
  pmessage("Writing datum index part ...\n");
  WriteDatumIndexPart(fd, offset + sizeof(int) * amount);
  pmessage("Writing phrase datum part ...\n");
  WritePhraseDatumPart(fd);
  pmessage("Finished!\n");

  /* 关闭码表文件 */
  close(fd);
}

/**
 * 分割词语数据串.
 * @param string 数据串
 * @retval phrase 词语串
 * @retval pinyin 拼音串
 * @retval frequency 频率串
 * @return 串是否合法
 */
bool MBCreater::BreakPhraseString(char *string, const char **phrase,
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
PhraseDatum *MBCreater::CreatePhraseDatum(const char *phrase,
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
void MBCreater::InsertDatumToTree(PhraseDatum *datum) {
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
std::list<PhraseLengthNode *> *MBCreater::SearchChildByIndex(
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
std::list<PhraseDatum *> *MBCreater::SearchChildByLength(
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
 * 写出纯索引部分.
 * (最大索引值)-->(索引值,最大长度)-->(长度,总孩子数)-->(汉字代理数组).
 * @param fd 文件描述字
 * @retval offset 数据索引部分的偏移量
 * @return 总词语数
 */
uint MBCreater::WritePureIndexPart(int fd, int *offset) {
  uint phrase_datum_amount = 0;

  std::list<PhraseIndexNode *> *index_list = &root_.data_;
  int8_t max_chars_proxy_index = index_list->back()->chars_proxy_index_;
  xwrite(fd, &max_chars_proxy_index, sizeof(max_chars_proxy_index));
  for (std::list<PhraseIndexNode *>::iterator iterator = index_list->begin();
       iterator != index_list->end();
       ++iterator) {
    int8_t chars_proxy_index = (*iterator)->chars_proxy_index_;
    xwrite(fd, &chars_proxy_index, sizeof(chars_proxy_index));
    std::list<PhraseLengthNode *> *length_list = &(*iterator)->data_;
    int max_chars_proxy_length = length_list->back()->chars_proxy_length_;
    xwrite(fd, &max_chars_proxy_length, sizeof(max_chars_proxy_length));
    for (std::list<PhraseLengthNode *>::iterator iterator = length_list->begin();
         iterator != length_list->end();
         ++iterator) {
      int chars_proxy_length = (*iterator)->chars_proxy_length_;
      xwrite(fd, &chars_proxy_length, sizeof(chars_proxy_length));
      std::list<PhraseDatum *> *datum_list = &(*iterator)->data_;
      uint phrase_datum_count = 0/*datum_list->size()*/;
      int data_offset = lseek(fd, sizeof(phrase_datum_count), SEEK_CUR);
      for (std::list<PhraseDatum *>::iterator iterator = datum_list->begin();
           iterator != datum_list->end();
           ++iterator) {
        xwrite(fd, (*iterator)->chars_proxy_,
               sizeof(CharsProxy) * chars_proxy_length);
        ++phrase_datum_count;
      }
      lseek(fd, data_offset - sizeof(phrase_datum_count), SEEK_SET);
      xwrite(fd, &phrase_datum_count, sizeof(phrase_datum_count));
      phrase_datum_amount += phrase_datum_count;
      *offset = lseek(fd, 0, SEEK_END);
    }
  }

  return phrase_datum_amount;
}

/**
 * 写出数据索引部分.
 * ()-->()-->()-->(偏移量)
 * @param fd 文件描述字
 * @param offset 词语数据部分的偏移量
 */
void MBCreater::WriteDatumIndexPart(int fd, int offset) {
  std::list<PhraseIndexNode *> *index_list = &root_.data_;
  for (std::list<PhraseIndexNode *>::iterator iterator = index_list->begin();
       iterator != index_list->end();
       ++iterator) {
    std::list<PhraseLengthNode *> *length_list = &(*iterator)->data_;
    for (std::list<PhraseLengthNode *>::iterator iterator = length_list->begin();
         iterator != length_list->end();
         ++iterator) {
      std::list<PhraseDatum *> *datum_list = &(*iterator)->data_;
      for (std::list<PhraseDatum *>::iterator iterator = datum_list->begin();
           iterator != datum_list->end();
           ++iterator) {
        PhraseDatum *datum = *iterator;
        xwrite(fd, &offset, sizeof(offset));
        offset += sizeof(datum->raw_data_length_) + datum->raw_data_length_;
      }
    }
  }
}

/**
 * 写出词语数据部分.
 * ()-->()-->()-->(数据长度,词语数据).
 * @param fd 文件描述字
 */
void MBCreater::WritePhraseDatumPart(int fd) {
  std::list<PhraseIndexNode *> *index_list = &root_.data_;
  for (std::list<PhraseIndexNode *>::iterator iterator = index_list->begin();
       iterator != index_list->end();
       ++iterator) {
    std::list<PhraseLengthNode *> *length_list = &(*iterator)->data_;
    for (std::list<PhraseLengthNode *>::iterator iterator = length_list->begin();
         iterator != length_list->end();
         ++iterator) {
      std::list<PhraseDatum *> *datum_list = &(*iterator)->data_;
      for (std::list<PhraseDatum *>::iterator iterator = datum_list->begin();
           iterator != datum_list->end();
           ++iterator) {
        PhraseDatum *datum = *iterator;
        xwrite(fd, &datum->raw_data_length_, sizeof(datum->raw_data_length_));
        xwrite(fd, datum->raw_data_, datum->raw_data_length_);
      }
    }
  }
}
