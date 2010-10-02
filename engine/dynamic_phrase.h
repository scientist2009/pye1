//
// C++ Interface: dynamic_phrase
//
// Description:
// 生成动态词语数据.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_DYNAMIC_PHRASE_H_
#define PYE_ENGINE_DYNAMIC_PHRASE_H_

#include "abstract_phrase.h"

/**
 * 动态词语类.
 */
class DynamicPhrase {
 public:
  DynamicPhrase();
  ~DynamicPhrase();

  void GetDynamicPhrase(const char *string, std::list<PhraseDatum *> *list);

 private:
  void GetDatePhrase(std::list<PhraseDatum *> *list);
  void GetTimePhrase(std::list<PhraseDatum *> *list);
  void GetWeekPhrase(std::list<PhraseDatum *> *list);
};

#endif  // PYE_ENGINE_DYNAMIC_PHRASE_H_
