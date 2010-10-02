//
// C++ Implementation: pinyin_parser
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pinyin_parser.h"
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "output.h"
#define PINYIN_MAX 6

/**
 * 拼音单元部件数组.
 * @note 为了保证拼音分析函数能够尽量输出正确的结果，
 * 你不应该随意调整本数组元素之间的顺序. \n
 */
PinyinUnitParts PinyinParser::parts_array_[] = {
  {"iang", MINOR_TYPE},  ///< 0x00 0
  {"iong", MINOR_TYPE},  ///< 0x01 1
  {"uang", MINOR_TYPE},  ///< 0x02 2
  {"ang", MAJIN_TYPE},  ///< 0x03 3
  {"eng", MAJIN_TYPE},  ///< 0x04 4
  {"ian", MINOR_TYPE},  ///< 0x05 5
  {"iao", MINOR_TYPE},  ///< 0x06 6
  {"ing", MINOR_TYPE},  ///< 0x07 7
  {"ong", MINOR_TYPE},  ///< 0x08 8
  {"uai", MINOR_TYPE},  ///< 0x09 9
  {"uan", MINOR_TYPE},  ///< 0x0a 10
  {"ai", MAJIN_TYPE},  ///< 0x0b 11
  {"an", MAJIN_TYPE},  ///< 0x0c 12
  {"ao", MAJIN_TYPE},  ///< 0x0d 13
  {"ch", MAJOR_TYPE},  ///< 0x0e 14
  {"ei", MAJIN_TYPE},  ///< 0x0f 15
  {"en", MAJIN_TYPE},  ///< 0x10 16
  {"er", ATOM_TYPE},  ///< 0x11 17
  {"ia", MINOR_TYPE},  ///< 0x12 18
  {"ie", MINOR_TYPE},  ///< 0x13 19
  {"in", MINOR_TYPE},  ///< 0x14 20
  {"iu", MINOR_TYPE},  ///< 0x15 21
  {"ou", MAJIN_TYPE},  ///< 0x16 22
  {"sh", MAJOR_TYPE},  ///< 0x17 23
  {"ua", MINOR_TYPE},  ///< 0x18 24
  {"ue", MINOR_TYPE},  ///< 0x19 25
  {"ui", MINOR_TYPE},  ///< 0x1a 26
  {"un", MINOR_TYPE},  ///< 0x1b 27
  {"uo", MINOR_TYPE},  ///< 0x1c 28
  {"ve", MINOR_TYPE},  ///< 0x1d 29
  {"zh", MAJOR_TYPE},  ///< 0x1e 30
  {"a", MAJIN_TYPE},  ///< 0x1f 31
  {"b", MAJOR_TYPE},  ///< 0x20 32
  {"c", MAJOR_TYPE},  ///< 0x21 33
  {"d", MAJOR_TYPE},  ///< 0x22 34
  {"e", MAJIN_TYPE},  ///< 0x23 35
  {"f", MAJOR_TYPE},  ///< 0x24 36
  {"g", MAJOR_TYPE},  ///< 0x25 37
  {"h", MAJOR_TYPE},  ///< 0x26 38
  {"i", MINOR_TYPE},  ///< 0x27 39
  {"j", MAJOR_TYPE},  ///< 0x28 40
  {"k", MAJOR_TYPE},  ///< 0x29 41
  {"l", MAJOR_TYPE},  ///< 0x2a 42
  {"m", MAJOR_TYPE},  ///< 0x2b 43
  {"n", MAJOR_TYPE},  ///< 0x2c 44
  {"o", MAJIN_TYPE},  ///< 0x2d 45
  {"p", MAJOR_TYPE},  ///< 0x2e 46
  {"q", MAJOR_TYPE},  ///< 0x2f 47
  {"r", MAJOR_TYPE},  ///< 0x30 48
  {"s", MAJOR_TYPE},  ///< 0x31 49
  {"t", MAJOR_TYPE},  ///< 0x32 50
  {"u", MINOR_TYPE},  ///< 0x33 51
  {"v", MINOR_TYPE},  ///< 0x34 52
  {"w", MAJOR_TYPE},  ///< 0x35 53
  {"x", MAJOR_TYPE},  ///< 0x36 54
  {"y", MAJOR_TYPE},  ///< 0x37 55
  {"z", MAJOR_TYPE},  ///< 0x38 56
  {NULL, ATOM_TYPE}
};

/**
 * 类构造函数.
 */
PinyinParser::PinyinParser() {
}

/**
 * 类析构函数.
 */
PinyinParser::~PinyinParser() {
}

/**
 * 分析拼音串.
 * @param pinyin 原始拼音串，e.g.<yumen,yu'men>
 * @retval chars_proxy 汉字代理数组
 * @retval length 汉字代理数组有效长度
 * @return 是否分析成功
 */
bool PinyinParser::ParsePinyin(const char *pinyin, CharsProxy **chars_proxy, int *length) {
  /* 申请足够内存 */
  size_t size = strlen(pinyin);
  *chars_proxy = new CharsProxy[size];

  /* 分析拼音串 */
  *length = -1;
  PinyinUnitAttribute type = ATOM_TYPE;
  const char *ptr = pinyin;
  while (*ptr != '\0') {
    int8_t index = SearchMatchablePinyinUnitParts(ptr);
    if (index != -1) {
      AppendPinyinUnitParts(*chars_proxy, length, index, &type);
      ptr += strlen((parts_array_ + index)->data);
    } else {
      ++ptr;
    }
  }
  ++(*length);

  return true;
}

/**
 * 恢复拼音串.
 * @param chars_proxy 汉字代理数组
 * @param length 汉字代理数组有效长度
 * @return 拼音串
 */
char *PinyinParser::UnparsePinyin(const CharsProxy *chars_proxy, int length) {
  /* 申请足够内存 */
  char *pinyin = (char *)malloc((PINYIN_MAX + 1) * length + 1);

  /* 恢复拼音串 */
  char *ptr = pinyin;
  for (int count = 0; count < length; ++count) {
    if ((chars_proxy + count)->minor_index_ != -1) {
      sprintf(ptr, "%s%s\'",
              (parts_array_ + (chars_proxy + count)->major_index_)->data,
              (parts_array_ + (chars_proxy + count)->minor_index_)->data);
    } else {
      sprintf(ptr, "%s\'",
              (parts_array_ + (chars_proxy + count)->major_index_)->data);
    }
    ptr += strlen(ptr);
  }
  if (ptr != pinyin)
    --ptr;
  *ptr = '\0';

  return pinyin;
}

/**
 * 获取拼音单元部件的索引值.
 * @param pinyin 字符串
 * @return 索引值
 */
int8_t PinyinParser::GetPinyinUnitPartsIndex(const char *pinyin) {
  return SearchMatchablePinyinUnitParts(pinyin);
}

/**
 * 获取拼音单元部件的总数.
 * @return 总数
 */
int8_t PinyinParser::GetPinyinUnitPartsAmount() {
  return N_ELEMENTS(parts_array_);
}

/**
 * 搜索拼音串所匹配的拼音单元部件的索引值.
 * @param pinyin 拼音串，e.g.<yumen,u'men,'men>
 * @return 索引值
 */
int8_t PinyinParser::SearchMatchablePinyinUnitParts(const char *pinyin) {
  PinyinUnitParts *parts = parts_array_;
  for (; parts->data != NULL; ++parts) {
    if (memcmp(pinyin, parts->data, strlen(parts->data)) == 0)
      break;
  }
  if (parts->data == NULL)
    return -1;
  return parts - parts_array_;
}

/**
 * 附加拼音单元部件到汉字代理数组.
 * @param chars_proxy 汉字代理数组
 * @retval offset 操作汉字代理数组的偏移量
 * @param parts_index 拼音单元部件的索引值
 * @retval type 前一个拼音单元部件属性
 */
void PinyinParser::AppendPinyinUnitParts(CharsProxy *chars_proxy,
                                         int *offset,
                                         int8_t parts_index,
                                         PinyinUnitAttribute *type) {
  PinyinUnitAttribute parts_type = (parts_array_ + parts_index)->type;
  switch (*type) {
    case ATOM_TYPE:
    case MINOR_TYPE:
    case MAJIN_TYPE:
      if (parts_type == ATOM_TYPE || parts_type & MAJOR_TYPE) {
        ++(*offset);
        (chars_proxy + *offset)->major_index_ = parts_index;
        *type = parts_type;
      } else {
        ptrace("discard pinyin unit parts \"%s\"\n",
               (parts_array_ + parts_index)->data);
      }
      break;
    case MAJOR_TYPE:
      if (parts_type & MINOR_TYPE) {
        (chars_proxy + *offset)->minor_index_ = parts_index;
        *type = ATOM_TYPE;
      } else {
        ++(*offset);
        (chars_proxy + *offset)->major_index_ = parts_index;
        *type = parts_type;
      }
      break;
    default:
      assert(false);
  }
}
