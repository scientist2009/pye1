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

#define N_ELEMENTS(ArrayName) \
            (sizeof(ArrayName)/sizeof((ArrayName)[0]))

#define DELETE_LIST_DATA(ListName,TypeName) \
do { \
  for (std::list<TypeName *>::iterator iterator = (ListName).begin(); \
       iterator != (ListName).end(); \
       ++iterator) \
    delete (*iterator); \
} while (0)

#endif  // PYE_ENGINE_GLOBAL_H_
