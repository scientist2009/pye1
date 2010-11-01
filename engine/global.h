//
// C++ Interface: global
//
// Description:
// 全局宏定义.
//
// Author: Jally <jallyx@163.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_GLOBAL_H_
#define PYE_ENGINE_GLOBAL_H_

#define N_ARRAY_ELEMENTS(ArrayName) \
    (sizeof(ArrayName)/sizeof((ArrayName)[0]))

#define STL_DELETE_DATA(Name,Type) \
do { \
  for (Type::iterator iterator = (Name).begin(); \
       iterator != (Name).end(); \
       ++iterator) \
    delete (*iterator); \
} while (0)

#define STL_FREE_DATA(Name,Type) \
do { \
  for (Type::iterator iterator = (Name).begin(); \
       iterator != (Name).end(); \
       ++iterator) \
    free(*iterator); \
} while (0)

#define STL_FREE_KEY(Name,Type) \
do { \
  for (Type::iterator iterator = (Name).begin(); \
       iterator != (Name).end(); \
       ++iterator) \
    free(iterator->first); \
} while (0)

#define STL_FREE_VALUE(Name,Type) \
do { \
  for (Type::iterator iterator = (Name).begin(); \
       iterator != (Name).end(); \
       ++iterator) \
    free(iterator->second); \
} while (0)

#endif  // PYE_ENGINE_GLOBAL_H_
