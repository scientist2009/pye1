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

/*
 * ==== 函数表(2010/10/31 16:05:09 星期日 下午) ====
 * 函数                含义                举例
 * $year              年(4位)             2010
 * $year_yy           年(2位)             10
 * $year_cn           年(中文4位)          二〇一〇
 * $year_yy_cn        年(中文2位)          一〇
 * $month             月                  10
 * $month_mm          月(2位)             10
 * $month_cn          月(中文)             十
 * $day               日                  31
 * $day_dd            日(2位)              31
 * $day_cn            日(中文)             三十一
 * $fullhour          时(24小时制)         16
 * $fullhour_hh       时(2位24小时制)       16
 * $fullhour_cn       时(中文24小时制)      十六
 * $halfhour          时(12小时制)          4
 * $halfhour_hh       时(2位12小时制)       04
 * $halfhour_cn       时(中文12小时制)       四
 * $minute            分                   5
 * $minute_mm         分(2位)              05
 * $minute_cn         分(中文)             五
 * $second            秒                   9
 * $second_ss         秒(2位)              09
 * $second_cn         秒(中文)             九
 * $weekday           星期                 0
 * $weekday_cn        星期(中文)            日
 * $ampm              AMPM                PM
 * $ampm_cn           上午下午             下午
 */

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
 * 获取实例对象.
 * @return 实例对象
 */
static DynamicPhrase *DynamicPhrase::GetInstance() {
  static DynamicPhrase instance;
  return &instance;
}

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

