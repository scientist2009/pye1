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

#include <string.h>
#include <map>
#include "abstract_phrase.h"

/**
 * 动态词语类.
 */
class DynamicPhrase {
 public:
  void CreateExpression(const char *config);

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
  typedef char *(*GenerateDatumFunc)();

  DynamicPhrase();
  ~DynamicPhrase();

  PhraseDatum *CreatePhraseDatum(const char *data) const;

  char *GenerateYear();
  char *GenerateYearYY();
  char *GenerateYearCN();
  char *GenerateYearYYCN();

  std::multimap<char *, char *, StringComparer> expression_;  ///< 词语表达式
  std::map<const char *, GenerateDatumFunc, StringComparer> function_;  ///< 词语函数
};

#endif  // PYE_ENGINE_DYNAMIC_PHRASE_H_
