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
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "global.h"
#include "output.h"
#include "pinyin_parser.h"
#include "wrapper.h"

/* 最大字符长度 */
#define CHARS_MAX 6

/**
 * 创建表达式.
 * @param config 动态词语配置文件名
 */
void DynamicPhrase::CreateExpression(const char *config) {
  /* 打开配置文件 */
  FILE *stream = fopen(config, "r");
  if (!stream) {
    pwarning("Fopen file \"%s\" failed, %s", config, strerror(errno));
    return;
  }

  /* 读取文件数据、分析并添加到映射表 */
  char *lineptr = NULL;
  size_t n = 0;
  while (getline(&lineptr, &n, stream) != -1) {
    strstrip(lineptr);
    if (*lineptr == '\0' || *lineptr == '#')
      continue;
    const char *ptr = strchr(lineptr, '=');
    if (!ptr || ptr == lineptr || *(ptr + 1) == '\0')
      continue;
    char *key = strndup(lineptr, ptr - lineptr);
    strchomp(key);
    char *value = strdup(ptr + 1);
    strchug(value);
    expression_.insert(std::pair<char *, char *>(key, value));
  }
  free(lineptr);

  /* 关闭文件 */
  fclose(stream);
}

/**
 * 清除表达式.
 */
void DynamicPhrase::ClearExpression() {
  /* 定义映射表类型 */
  typedef std::multimap<char *, char *, StringComparer> ExpressionMultimap;

  STL_FREE_KEY(expression_, ExpressionMultimap);
  STL_FREE_VALUE(expression_, ExpressionMultimap);
  expression_.clear();
}

/**
 * 获取动态词语数据.
 * @param string 词语索引串
 * @param list 词语数据链表
 */
void DynamicPhrase::GetDynamicPhrase(const char *string,
                                     std::list<PhraseDatum *> *list) const {
  /* 定义迭代器类型 */
  typedef std::multimap<char *, char *, StringComparer>::const_iterator
      ExpressionIterator;

  /* 获取所有匹配项 */
  std::pair<ExpressionIterator, ExpressionIterator> pair =
      expression_.equal_range((char * const)string);
  /* 创建词语对象并加入链表 */
  for (ExpressionIterator iterator = pair.first;
       iterator != pair.second;
       ++iterator) {
    PhraseDatum *phrase_datum = CreatePhraseDatum(iterator->second);
    if (phrase_datum)
      list->push_back(phrase_datum);
  }
}

/**
 * 获取实例对象.
 * @return 实例对象
 */
DynamicPhrase *DynamicPhrase::GetInstance() {
  static DynamicPhrase instance;
  return &instance;
}

/**
 * 类构造函数.
 */
DynamicPhrase::DynamicPhrase() {
  RegisterFunction();
}

/**
 * 类析构函数.
 */
DynamicPhrase::~DynamicPhrase() {
  /* 定义映射表类型 */
  typedef std::multimap<char *, char *, StringComparer> ExpressionMultimap;

  STL_FREE_KEY(expression_, ExpressionMultimap);
  STL_FREE_VALUE(expression_, ExpressionMultimap);
}

/**
 * 注册动态数据生成函数.
 */
void DynamicPhrase::RegisterFunction() {
  function_["$year"] = &DynamicPhrase::GenerateYear;
  function_["$year_yy"] = &DynamicPhrase::GenerateYearYY;
  function_["$year_cn"] = &DynamicPhrase::GenerateYearCN;
  function_["$year_yy_cn"] = &DynamicPhrase::GenerateYearYYCN;
  function_["$month"] = &DynamicPhrase::GenerateMonth;
  function_["$month_mm"] = &DynamicPhrase::GenerateMonthMM;
  function_["$month_cn"] = &DynamicPhrase::GenerateMonthCN;
  function_["$day"] = &DynamicPhrase::GenerateDay;
  function_["$day_dd"] = &DynamicPhrase::GenerateDayDD;
  function_["$day_cn"] = &DynamicPhrase::GenerateDayCN;
  function_["$fullhour"] = &DynamicPhrase::GenerateFullhour;
  function_["$fullhour_hh"] = &DynamicPhrase::GenerateFullhourHH;
  function_["$fullhour_cn"] = &DynamicPhrase::GenerateFullhourCN;
  function_["$halfhour"] = &DynamicPhrase::GenerateHalfhour;
  function_["$halfhour_hh"] = &DynamicPhrase::GenerateHalfhourHH;
  function_["$halfhour_cn"] = &DynamicPhrase::GenerateHalfhourCN;
  function_["$minute"] = &DynamicPhrase::GenerateMinute;
  function_["$minute_mm"] = &DynamicPhrase::GenerateMinuteMM;
  function_["$minute_cn"] = &DynamicPhrase::GenerateMinuteCN;
  function_["$second"] = &DynamicPhrase::GenerateSecond;
  function_["$second_ss"] = &DynamicPhrase::GenerateSecondSS;
  function_["$second_cn"] = &DynamicPhrase::GenerateSecondCN;
  function_["$weekday"] = &DynamicPhrase::GenerateWeekday;
  function_["$weekday_cn"] = &DynamicPhrase::GenerateWeekdayCN;
  function_["$ampm"] = &DynamicPhrase::GenerateAMPM;
  function_["$ampm_cn"] = &DynamicPhrase::GenerateAMPMCN;
}

/**
 * 创建词语数据.
 * @param expression 数据表达式
 * @return 词语数据
 */
PhraseDatum *DynamicPhrase::CreatePhraseDatum(const char *expression) const {
  /* 定义迭代器类型 */
  typedef std::map<const char *, GenerateDatumFunc, StringComparer>::
      const_iterator FunctionIterator;

  /* 分解数据并替换动态部分 */
  size_t debris_data_length = 0;
  std::list<char *> debris_list;
  char *data_expression = strdup(expression);
  const char *pptr = data_expression;
  for (char *ptr = data_expression; *ptr != '\0'; ++ptr) {
    if (*ptr == '$') {
      char *aptr = ptr + 1;
      for (; *aptr != '\0'; ++aptr) {
        if (!islower(*aptr) && *aptr != '_')
          break;
      }
      if (aptr == ptr + 1)
        continue;
      char ch = *aptr;
      *aptr = '\0';
      FunctionIterator iterator = function_.find(ptr);
      if (iterator != function_.end()) {
        if (pptr != ptr) {
          char *debris = strndup(pptr, ptr - pptr);
          debris_data_length += ptr - pptr;
          debris_list.push_back(debris);
        }
        GenerateDatumFunc function = iterator->second;
        char *debris = (((DynamicPhrase *)this)->*function)();
        if (debris) {
          debris_data_length += strlen(debris);
          debris_list.push_back(debris);
        }
        pptr = ptr = aptr;
      }
      *aptr = ch;
    }
  }
  if (pptr == data_expression) {
    debris_data_length += strlen(data_expression);
    debris_list.push_back(data_expression);
  } else {
    if (*pptr != '\0') {
      char *debris = strdup(pptr);
      debris_data_length += strlen(debris);
      debris_list.push_back(debris);
    }
    free(data_expression);
  }

  /* 创建词语数据 */
  PhraseDatum *phrase_datum = new PhraseDatum;
  PinyinParser pinyin_parser;
  pinyin_parser.ParsePinyin("jl", &phrase_datum->chars_proxy_,
                            &phrase_datum->chars_proxy_length_);
  phrase_datum->raw_data_ = malloc(debris_data_length + 1);
  char *ptr = (char *)phrase_datum->raw_data_;
  for (std::list<char *>::iterator iterator = debris_list.begin();
       iterator != debris_list.end();
       ++iterator) {
    strcpy(ptr, *iterator);
    ptr += strlen(ptr);
  }
  phrase_datum->raw_data_length_ = debris_data_length;
  phrase_datum->phrase_data_offset_ = InvalidPhraseType;

  /* 释放数据 */
  STL_FREE_DATA(debris_list, std::list<char *>);

  return phrase_datum;
}

/**
 * 生成年(4位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateYear() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%04d", (timeinfo.tm_year + 1900) % 10000);
  return datum;
}

/**
 * 生成年(2位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateYearYY() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%02d", (timeinfo.tm_year + 1900) % 100);
  return datum;
}

/**
 * 生成年(中文4位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateYearCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToSimpleNumericCN(timeinfo.tm_year + 1900, 4);
}

/**
 * 生成年(中文2位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateYearYYCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToSimpleNumericCN(timeinfo.tm_year + 1900, 2);
}

/**
 * 生成月.
 * @return 数据串
 */
char *DynamicPhrase::GenerateMonth() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_mon + 1);
  return datum;
}

/**
 * 生成月(2位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateMonthMM() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%02d", timeinfo.tm_mon + 1);
  return datum;
}

/**
 * 生成月(中文).
 * @return 数据串
 */
char *DynamicPhrase::GenerateMonthCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToComplexNumericCN(timeinfo.tm_mon + 1);
}

/**
 * 生成日.
 * @return 数据串
 */
char *DynamicPhrase::GenerateDay() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_mday);
  return datum;
}

/**
 * 生成日(2位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateDayDD() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%02d", timeinfo.tm_mday);
  return datum;
}

/**
 * 生成日(中文).
 * @return 数据串
 */
char *DynamicPhrase::GenerateDayCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToComplexNumericCN(timeinfo.tm_mday);
}

/**
 * 生成时(24小时制).
 * @return 数据串
 */
char *DynamicPhrase::GenerateFullhour() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_hour);
  return datum;
}

/**
 * 生成时(2位24小时制).
 * @return 数据串
 */
char *DynamicPhrase::GenerateFullhourHH() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%02d", timeinfo.tm_hour);
  return datum;
}

/**
 * 生成时(中文24小时制).
 * @return 数据串
 */
char *DynamicPhrase::GenerateFullhourCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToComplexNumericCN(timeinfo.tm_hour);
}

/**
 * 生成时(12小时制).
 * @return 数据串
 */
char *DynamicPhrase::GenerateHalfhour() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  if (timeinfo.tm_hour >= 12)
    timeinfo.tm_hour -= 12;
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_hour);
  return datum;
}

/**
 * 生成时(2位12小时制).
 * @return 数据串
 */
char *DynamicPhrase::GenerateHalfhourHH() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  if (timeinfo.tm_hour >= 12)
    timeinfo.tm_hour -= 12;
  char *datum = NULL;
  asprintf(&datum, "%02d", timeinfo.tm_hour);
  return datum;
}

/**
 * 生成时(中文12小时制).
 * @return 数据串
 */
char *DynamicPhrase::GenerateHalfhourCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  if (timeinfo.tm_hour >= 12)
    timeinfo.tm_hour -= 12;
  return ToComplexNumericCN(timeinfo.tm_hour);
}

/**
 * 生成分.
 * @return 数据串
 */
char *DynamicPhrase::GenerateMinute() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_min);
  return datum;
}

/**
 * 生成分(2位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateMinuteMM() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%02d", timeinfo.tm_min);
  return datum;
}

/**
 * 生成分(中文).
 * @return 数据串
 */
char *DynamicPhrase::GenerateMinuteCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToComplexNumericCN(timeinfo.tm_min);
}

/**
 * 生成秒.
 * @return 数据串
 */
char *DynamicPhrase::GenerateSecond() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_sec);
  return datum;
}

/**
 * 生成秒(2位).
 * @return 数据串
 */
char *DynamicPhrase::GenerateSecondSS() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%02d", timeinfo.tm_sec);
  return datum;
}

/**
 * 生成秒(中文).
 * @return 数据串
 */
char *DynamicPhrase::GenerateSecondCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  return ToComplexNumericCN(timeinfo.tm_sec);
}

/**
 * 生成星期.
 * @return 数据串
 */
char *DynamicPhrase::GenerateWeekday() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  char *datum = NULL;
  asprintf(&datum, "%d", timeinfo.tm_wday);
  return datum;
}

/**
 * 生成星期(中文).
 * @return 数据串
 */
char *DynamicPhrase::GenerateWeekdayCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  const char *datum = ToWeekdayCN(timeinfo.tm_wday);
  return strdup(datum);
}

/**
 * 生成AM/PM.
 * @return 数据串
 */
char *DynamicPhrase::GenerateAMPM() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  const char *datum = ToAMPM(timeinfo.tm_hour);
  return strdup(datum);
}

/**
 * 生成上午/下午.
 * @return 数据串
 */
char *DynamicPhrase::GenerateAMPMCN() {
  time_t tt = time(NULL);
  struct tm timeinfo;
  localtime_r(&tt, &timeinfo);
  const char *datum = ToAMPMCN(timeinfo.tm_hour);
  return strdup(datum);
}

/**
 * 转换简单数值.
 * @param number 数值
 * @param amount 总位数
 * @return 数值串
 */
char *DynamicPhrase::ToSimpleNumericCN(int number, int amount) {
  /* 获取数值串 */
  char buffer[12];  // 1+10 +1 =12
  snprintf(buffer, sizeof(buffer), "%010d", number);

  /* 转换中文数值串 */
  char *datum = (char *)malloc(CHARS_MAX * amount + 1);
  char *datum_ptr = datum;
  for (const char *ptr = buffer + strlen(buffer) - amount; *ptr != 0; ++ptr) {
    strcpy(datum_ptr, ToDigitCN(*ptr));
    datum_ptr += strlen(datum_ptr);
  }

  return datum;
}

/**
 * 转换可读数值.
 * @param number 数值
 * @return 数值串
 */
char *DynamicPhrase::ToComplexNumericCN(int number) {
  /* 获取数值串 */
  char buffer[12];  // 1+10 +1 =12
  snprintf(buffer, sizeof(buffer), "%d", number);

  /* 转换中文数值串 */
  size_t length = strlen(buffer);
  char *datum = (char *)malloc(CHARS_MAX * length * 2 + 1);
  char *datum_ptr = datum;
  bool zero_digit = false;  // 0数位标记
  uint decimal = length;  // 当前位数
  for (const char *ptr = buffer; *ptr != '\0'; ++ptr, --decimal) {
    if (*ptr != '0') {
      /* e.g. 9009,909 */
      if (zero_digit) {
        strcpy(datum_ptr, "零");
        datum_ptr += strlen(datum_ptr);
        zero_digit = false;
      }
      /* e.g. 19999,19 */
      if (decimal != length || *ptr != '1' || length % 4 != 2) {
        strcpy(datum_ptr, ToDigitCN(*ptr));
        datum_ptr += strlen(datum_ptr);
      }
      strcpy(datum_ptr, ToDecimalCN(decimal));
      datum_ptr += strlen(datum_ptr);
    } else {
      zero_digit = true;
      /* e.g. 109999 */
      if ((decimal - 1) % 4 == 0) {
        strcpy(datum_ptr, ToDecimalCN(decimal));
        datum_ptr += strlen(datum_ptr);
        zero_digit = false;
      }
    }
  }
  /* 特例, e.g. 0 */
  if (length == 1 && buffer[0] == '0')
    strcpy(datum, "零");

  return datum;
}

/**
 * 转换中文数字.
 * @param digit 数字
 * @return 数字串
 */
const char *DynamicPhrase::ToDigitCN(char digit) {
  switch (digit) {
    case '0': return "〇";
    case '1': return "一";
    case '2': return "二";
    case '3': return "三";
    case '4': return "四";
    case '5': return "五";
    case '6': return "六";
    case '7': return "七";
    case '8': return "八";
    case '9': return "九";
  }
  return "";
}

/**
 * 转换中文单位.
 * 参考孙子算经.
 * @param decimal 当前位数
 * @return 单位串
 */
const char *DynamicPhrase::ToDecimalCN(int decimal) {
  switch ((decimal - 1) % 4) {
  case 0:
    switch (decimal / 4) {
      case 1: return "万";
      case 2: return "亿";
      case 3: return "兆";
      case 4: return "京";
      case 5: return "垓";
      case 6: return "秭";
      case 7: return "穰";
      case 8: return "沟";
      case 9: return "涧";
      case 10: return "正";
      case 11: return "载";
    }
    return "";
  case 1: return "十";
  case 2: return "百";
  case 3: return "千";
  }
  return "";
}

/**
 * 转换中文星期.
 * @param weekday 星期
 * @return 星期串
 */
const char *DynamicPhrase::ToWeekdayCN(int weekday) {
  switch (weekday) {
    case 0: return "日";
    case 1: return "一";
    case 2: return "二";
    case 3: return "三";
    case 4: return "四";
    case 5: return "五";
    case 6: return "六";
  }
  return "";
}

/**
 * 转换AM/PM.
 * @param 小时数
 * @return AM/PM串
 */
const char *DynamicPhrase::ToAMPM(int hour) {
  if (hour < 12)
    return "AM";
  else
    return "PM";
  return "";
}

/**
 * 转换中文上午/下午.
 * @param 小时数
 * @return 上午/下午串
 */
const char *DynamicPhrase::ToAMPMCN(int hour) {
  if (hour < 12)
    return "上午";
  else
    return "下午";
  return "";
}
