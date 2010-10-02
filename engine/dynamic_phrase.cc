//
// C++ Implementation: dynamic_phrase
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dynamic_phrase.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"

/**
 * 类构造函数.
 */
DynamicPhrase::DynamicPhrase() {
}

/**
 * 类析构函数.
 */
DynamicPhrase::~DynamicPhrase() {
}

/**
 * 获取动态词语数据.
 * @param string 源串
 * @retval list 词语数据链表
 */
void DynamicPhrase::GetDynamicPhrase(const char *string,
                                     std::list<PhraseDatum *> *list) {
  if (strcmp(string, "rq") == 0)
    GetDatePhrase(list);
  if (strcmp(string, "sj") == 0)
    GetTimePhrase(list);
  if (strcmp(string, "xq") == 0 || strcmp(string, "lb") == 0)
    GetWeekPhrase(list);
}

/**
 * 获取日期动态词语数据.
 * @retval list 词语数据链表
 */
void DynamicPhrase::GetDatePhrase(std::list<PhraseDatum *> *list) {
  /* 预备工作 */
  CharsProxy chars_proxy[2];
  PinyinParser pinyin_parser;
  chars_proxy[0].major_index_ = pinyin_parser.GetPinyinUnitPartsIndex("r");
  chars_proxy[1].major_index_ = pinyin_parser.GetPinyinUnitPartsIndex("q");
  const int length = N_ELEMENTS(chars_proxy);
  time_t tt = time(NULL);
  struct tm *tm = localtime(&tt);

  /* 2009年11月2日 */
  PhraseDatum *phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "%d年%d月%d日",
                                            tm->tm_year + 1900,
                                            tm->tm_mon + 1,
                                            tm->tm_mday);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 2009-11-02 */
  phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "%04d-%02d-%02d",
                                            tm->tm_year + 1900,
                                            tm->tm_mon + 1,
                                            tm->tm_mday);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 2009.11.02 */
  phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "%04d.%02d.%02d",
                                            tm->tm_year + 1900,
                                            tm->tm_mon + 1,
                                            tm->tm_mday);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 11/02/2009 */
  phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "%02d/%02d/%04d",
                                            tm->tm_mon + 1,
                                            tm->tm_mday,
                                            tm->tm_year + 1900);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;
}

/**
 * 获取时间动态词语数据.
 * @retval list 词语数据链表
 */
void DynamicPhrase::GetTimePhrase(std::list<PhraseDatum *> *list) {
  /* 预备工作 */
  CharsProxy chars_proxy[2];
  PinyinParser pinyin_parser;
  chars_proxy[0].major_index_ = pinyin_parser.GetPinyinUnitPartsIndex("s");
  chars_proxy[1].major_index_ = pinyin_parser.GetPinyinUnitPartsIndex("j");
  const int length = N_ELEMENTS(chars_proxy);
  time_t tt = time(NULL);
  struct tm *tm = localtime(&tt);

  /* 12时45分27秒 */
  PhraseDatum *phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "%d时%d分%d秒",
                                            tm->tm_hour,
                                            tm->tm_min,
                                            tm->tm_sec);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 12:45:27 */
  phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "%02d:%02d:%02d",
                                            tm->tm_hour,
                                            tm->tm_min,
                                            tm->tm_sec);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;
}

/**
 * 获取星期动态词语数据.
 * @retval list 词语数据链表
 */
void DynamicPhrase::GetWeekPhrase(std::list<PhraseDatum *> *list) {
  /* 预备工作 */
  CharsProxy chars_proxy[2];
  PinyinParser pinyin_parser;
  chars_proxy[0].major_index_ = pinyin_parser.GetPinyinUnitPartsIndex("x");
  chars_proxy[1].major_index_ = pinyin_parser.GetPinyinUnitPartsIndex("q");
  const int length = N_ELEMENTS(chars_proxy);
  time_t tt = time(NULL);
  struct tm *tm = localtime(&tt);

  /* 获取数字的汉字表示 */
  const char *week = "";
  switch (tm->tm_wday) {
    case 0: week = "日"; break;
    case 1: week = "一"; break;
    case 2: week = "二"; break;
    case 3: week = "三"; break;
    case 4: week = "四"; break;
    case 5: week = "五"; break;
    case 6: week = "六"; break;
  }

  /* 星期一 */
  PhraseDatum *phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "星期%s", week);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 礼拜一 */
  phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "礼拜%s", week);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 周一 */
  phrase_datum = new PhraseDatum;
  list->push_back(phrase_datum);
  phrase_datum->chars_proxy_ = new CharsProxy[length];
  memcpy(phrase_datum->chars_proxy_, chars_proxy, sizeof(chars_proxy));
  phrase_datum->chars_proxy_length_ = length;
  phrase_datum->raw_data_length_ = asprintf((char **)&phrase_datum->raw_data_,
                                            "周%s", week);
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;
}
