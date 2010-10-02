//
// C++ Interface: pinyin_editor
//
// Description:
// 拼音编辑器，以词语管理者为核心，借助其功能完成拼音到词语数据的转换.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PYE_ENGINE_PINYIN_EDITOR_H_
#define PYE_ENGINE_PINYIN_EDITOR_H_

#include "phrase_manager.h"
#include <string>

/**
 * 内部拼音纠错对.
 */
typedef struct _InnerMendPinyinPair {
  const char *raw;  ///< 原始串
  const char *mend;  ///< 纠错串
} InnerMendPinyinPair;

/**
 * 拼音编辑器.
 */
class PinyinEditor {
 public:
  PinyinEditor(const PhraseManager *phrase_manager);
  ~PinyinEditor();

  bool IsEmpty();
  bool SetEditorMode(bool zh);
  bool GetEditorMode();
  void MoveCursorPoint(int offset);
  int GetCursorPoint();
  void InsertPinyinKey(char ch);
  void DeletePinyinKey();
  void BackspacePinyinKey();
  bool RevokeSelectedPhrase();
  void GetRawText(char **text, int *len);
  void GetCommitText(char **text, int *len);
  void GetPreeditText(char **text, int *len);
  void GetAuxiliaryText(char **text, int *len);
  void GetPagePhrase(int pagesize, std::list<const PhraseDatum *> *list);
  void GetDynamicPhrase(std::list<const PhraseDatum *> *list);
  const PhraseDatum *GetComposePhrase();
  void SelectCachePhrase(const PhraseDatum *datum);
  void DeletePhraseData(const PhraseDatum *datum);
  void FeedbackSelectedPhrase();
  bool IsFinishTask();
  void StopTask();
  int GetPhraseOffset();

 private:
  PhraseDatum *CreateUserPhrase();
  PhraseProxyStorage *SearchPreferPhrase();

  void CreateCharsProxy();
  void LookupPhraseProxy();
  int FinishCharsOffset();
  bool IsExistCachePhrase(const PhraseDatum *datum);

  char *AmendPinyinString(
            const char *string,
            const std::list<OuterMendPinyinPair *> *mend_pair_table);
  char *AmendPinyinString(const char *string,
                          const InnerMendPinyinPair *mend_pair_table);

  void Clear();
  void ClearCharsProxy();
  void ClearAcceptedPhraseList();
  void ClearCachePhraseList();
  void ClearPhraseStorageList();

  bool editor_mode_;  ///< 当前编辑模式;true 中文,false 英文
  int cursor_point_;  ///< 当前光标位置
  std::string pinyin_table_;  ///< 待查询拼音表
  CharsProxy *chars_proxy_;  ///< 汉字代理数组
  int chars_proxy_length_;  ///< 汉字代理数组长度
  std::list<PhraseDatum *> accepted_phrase_list_;  ///< 已接受词语链表
  std::list<PhraseDatum *> cache_phrase_list_;  ///< 缓冲词语链表

  const PhraseManager *phrase_manager_;  ///< 词语管理者
  std::list<PhraseProxyStorage *> *phrase_storage_list_;  ///< 词语储存点链表

  static InnerMendPinyinPair mend_pair_table_[];  ///< 内置拼音修正表
};

#endif  // PYE_ENGINE_PINYIN_EDITOR_H_
