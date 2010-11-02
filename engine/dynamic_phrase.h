//
// C++ Interface: dynamic_phrase
//
// Description:
// 生成动态词语数据.
// 动态词语配置文件格式: 索引串 = 词语数据(内置函数)
// e.g.: rq = $year/$month/$day  ==>  output: 2009/5/10
//       year = $year_cn  ==>  output: 二〇〇九
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_DYNAMIC_PHRASE_H_
#define PYE_ENGINE_DYNAMIC_PHRASE_H_

#include <string.h>
#include <map>
#include "abstract_phrase.h"

/*
 * ==== 函数表(2009/05/10 20:31:12 星期日 下午) ====
 * 函数                含义                举例
 * $year              年(4位)             2009
 * $year_yy           年(2位)             09
 * $year_cn           年(中文4位)          二〇〇九
 * $year_yy_cn        年(中文2位)          〇九
 * $month             月                  5
 * $month_mm          月(2位)             05
 * $month_cn          月(中文)             五
 * $day               日                  10
 * $day_dd            日(2位)              10
 * $day_cn            日(中文)             十
 * $fullhour          时(24小时制)         20
 * $fullhour_hh       时(2位24小时制)       20
 * $fullhour_cn       时(中文24小时制)      二十
 * $halfhour          时(12小时制)          8
 * $halfhour_hh       时(2位12小时制)       08
 * $halfhour_cn       时(中文12小时制)       八
 * $minute            分                   31
 * $minute_mm         分(2位)              31
 * $minute_cn         分(中文)             三十一
 * $second            秒                   12
 * $second_ss         秒(2位)              12
 * $second_cn         秒(中文)             十二
 * $weekday           星期                 0
 * $weekday_cn        星期(中文)            日
 * $ampm              AMPM                PM
 * $ampm_cn           上午下午             下午
 */

/**
 * 动态词语类.
 */
class DynamicPhrase {
 public:
  /* 外部接口 */
  void CreateExpression(const char *config);
  void ClearExpression();

  void GetDynamicPhrase(const char *string,
                        std::list<PhraseDatum *> *list) const;

  static DynamicPhrase *GetInstance();

 private:
  /**
   * 字符串比较器.
   */
  struct StringComparer {
    bool operator()(const char *s1, const char *s2) const {
      return strcmp(s1, s2) < 0;
    }
  };

  /**
   * 数据生成函数.
   */
  typedef char *(DynamicPhrase::*GenerateDatumFunc)();

  DynamicPhrase();
  ~DynamicPhrase();

  void RegisterFunction();

  PhraseDatum *CreatePhraseDatum(const char *expression) const;

  char *GenerateYear();
  char *GenerateYearYY();
  char *GenerateYearCN();
  char *GenerateYearYYCN();
  char *GenerateMonth();
  char *GenerateMonthMM();
  char *GenerateMonthCN();
  char *GenerateDay();
  char *GenerateDayDD();
  char *GenerateDayCN();
  char *GenerateFullhour();
  char *GenerateFullhourHH();
  char *GenerateFullhourCN();
  char *GenerateHalfhour();
  char *GenerateHalfhourHH();
  char *GenerateHalfhourCN();
  char *GenerateMinute();
  char *GenerateMinuteMM();
  char *GenerateMinuteCN();
  char *GenerateSecond();
  char *GenerateSecondSS();
  char *GenerateSecondCN();
  char *GenerateWeekday();
  char *GenerateWeekdayCN();
  char *GenerateAMPM();
  char *GenerateAMPMCN();

  char *ToSimpleNumericCN(int number, int amount);
  char *ToComplexNumericCN(int number);

  const char *ToDigitCN(char digit);
  const char *ToDecimalCN(int decimal);
  const char *ToWeekdayCN(int weekday);
  const char *ToAMPM(int hour);
  const char *ToAMPMCN(int hour);

  std::multimap<char *, char *, StringComparer> expression_;  ///< 词语表达式
  std::map<const char *, GenerateDatumFunc, StringComparer> function_;  ///< 词语函数
};

#endif  // PYE_ENGINE_DYNAMIC_PHRASE_H_
