//
// C++ Interface: pinyin_parser
//
// Description:
// 将拼音串转化为汉字代理数组.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_PINYIN_PARSER_H_
#define PYE_ENGINE_PINYIN_PARSER_H_

#include <stdint.h>

/**
 * 拼音单元属性.
 */
typedef enum {
  ATOM_TYPE = 0,  ///< 自成一体，不与任何单元结合
  MAJOR_TYPE = 1 << 0,  ///< 在汉字拼音中只能处于第一个位置
  MINOR_TYPE = 1 << 1,  ///< 在汉字拼音中只能处于第二个位置
  MAJIN_TYPE = MAJOR_TYPE | MINOR_TYPE,  ///< 在汉字拼音中的位置不受限制
} PinyinUnitAttribute;

/**
 * 拼音单元部件.
 */
typedef struct {
  const char *data;  ///< 拼音单元串 *
  PinyinUnitAttribute type;  ///< 拼音单元属性
} PinyinUnitParts;

/**
* 汉字代理.
* 将汉字拼音分解为两个部件，并将部件转换为其在拼音单元部件数组中的索引值; \n
* (-1)代表本部分没有索引值，也即不存在，e.g.<er[xx,-1]>. \n
*/
class CharsProxy {
 public:
  CharsProxy() : major_index_(-1), minor_index_(-1) {}
  ~CharsProxy() {}

  int8_t major_index_;  ///< 第一部分的索引值
  int8_t minor_index_;  ///< 第二部分的索引值
};

/**
 * 拼音分析者.
 */
class PinyinParser {
 public:
  PinyinParser();
  ~PinyinParser();

  bool ParsePinyin(const char *pinyin, CharsProxy **chars_proxy, int *length);
  char *UnparsePinyin(const CharsProxy *chars_proxy, int length);
  int8_t GetPinyinUnitPartsIndex(const char *pinyin);
  int8_t GetPinyinUnitPartsAmount();

 private:
  int8_t SearchMatchablePinyinUnitParts(const char *pinyin);
  void AppendPinyinUnitParts(CharsProxy *chars_proxy, int *offset,
                             int8_t parts_index, PinyinUnitAttribute *type);

  static PinyinUnitParts parts_array_[];  ///< 拼音单元部件数组
};

#endif  // PYE_ENGINE_PINYIN_PARSER_H_
