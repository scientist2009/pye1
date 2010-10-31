//
// C++ Implementation: pinyin_editor
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pinyin_editor.h"
#include <string.h>
#include <algorithm>
#include "dynamic_phrase.h"

/**
 * 内置拼音修正表.
 * 这张表能够起到什么作用？ \n
 * e.g.:
 * geren-->g'er'en，我相信你一定不希望得到这个结果吧！ \n
 * 让我们调用这张表先对它做一次预处理！ \n
 * geren-->'ge'ren-->'g'e'r'en，这个结果应该可以接受了！ \n
 * \n
 * 已知bug: \n
 * zhongou-->zh'ong'ou，中欧(此为正确分解) \n
 * zhongou-->zhon'gou-->zh'g'ou，???(本程序处理结果) \n
 */
InnerMendPinyinPair PinyinEditor::mend_pair_table_[] = {
  {"ga", "\'ga"},
  {"ge", "\'ge"},
  {"gou", "\'gou"},
  {"gong", "\'gong"},
  {"gu", "\'gu"},
  {"na", "\'na"},
  {"ne", "\'ne"},
  {"ni", "\'ni"},
  {"nou", "\'nou"},
  {"nong", "\'nong"},
  {"nu", "\'nu"},
  {"nv", "\'nv"},
  {"ran", "\'ran"},
  {"rao", "\'rao"},
  {"re", "\'re"},
  {"ri", "\'ri"},
  {"rou", "\'rou"},
  {"rong", "\'rong"},
  {"ru", "\'ru"},
  {"aou", "a\'ou"},
  {"uou", "u\'ou"},
  {"iai", "i\'ai"},
  {"iei", "i\'ei"},
  {"uei", "u\'ei"},
  {"vei", "v\'ei"},
  {NULL, NULL}
};

/**
 * 类构造函数.
 */
PinyinEditor::PinyinEditor(const PhraseManager *phrase_manager)
    : editor_mode_(true), cursor_point_(0), chars_proxy_(NULL),
      chars_proxy_length_(0), phrase_manager_(phrase_manager),
      phrase_storage_list_(NULL) {
}

/**
 * 类析构函数.
 */
PinyinEditor::~PinyinEditor() {
  Clear();
}

/**
 * 拼音编辑器是否为空.
 * @return BOOL
 */
bool PinyinEditor::IsEmpty() {
  return pinyin_table_.empty();
}

/**
 * 设置编辑器的当前模式.
 * 主要用于避免在英文输入模式下，编辑器依然会查询词语。 \n
 * @param zh 模式;true 中文,false 英文
 * @return 执行状况
 */
bool PinyinEditor::SetEditorMode(bool zh) {
  if (!pinyin_table_.empty())
    return false;
  editor_mode_ = zh;
  return true;
}

/**
* 获取编辑器的当前模式.
* @return 模式;true 中文,false 英文
*/
bool PinyinEditor::GetEditorMode() {
  return editor_mode_;
}

/**
 * 移动当前光标点.
 * @param offset 偏移量
 */
void PinyinEditor::MoveCursorPoint(int offset) {
  cursor_point_ += offset;
  size_t length = pinyin_table_.size();
  if (cursor_point_ < 0)
    cursor_point_ = 0;
  else if ((size_t)cursor_point_ > length)
    cursor_point_ = length;
}

/**
 * 获取当前光标点.
 * @return 当前光标点
 */
int PinyinEditor::GetCursorPoint() {
  return cursor_point_;
}

/**
 * 插入新的拼音字符.
 * @param ch 字符
 */
void PinyinEditor::InsertPinyinKey(char ch) {
  /* 将字符插入待查询拼音表 */
  pinyin_table_.insert(cursor_point_, 1, ch);
  ++cursor_point_;
  /* 清空必要缓冲数据 */
  ClearCharsProxy();
  ClearAcceptedPhraseList();
  ClearCachePhraseList();
  ClearPhraseStorageList();
  /* 创建汉字代理数组 */
  CreateCharsProxy();
  /* 查询词语代理 */
  LookupPhraseProxy();
}

/**
 * 向前删除拼音字符.
 */
void PinyinEditor::DeletePinyinKey() {
  /* 移除字符 */
  size_t length = pinyin_table_.size();
  if ((size_t)cursor_point_ == length)
    return;
  pinyin_table_.erase(cursor_point_, 1);
  /* 清空必要缓冲数据 */
  ClearCharsProxy();
  ClearAcceptedPhraseList();
  ClearCachePhraseList();
  ClearPhraseStorageList();
  /* 创建汉字代理数组 */
  CreateCharsProxy();
  /* 查询词语代理 */
  LookupPhraseProxy();
}

/**
 * 向后删除拼音索引字符.
 */
void PinyinEditor::BackspacePinyinKey() {
  /* 移除字符 */
  if (cursor_point_ == 0)
    return;
  --cursor_point_;
  pinyin_table_.erase(cursor_point_, 1);
  /* 清空必要缓冲数据 */
  ClearCharsProxy();
  ClearAcceptedPhraseList();
  ClearCachePhraseList();
  ClearPhraseStorageList();
  /* 创建汉字代理数组 */
  CreateCharsProxy();
  /* 查询词语代理 */
  LookupPhraseProxy();
}

/**
 * 取消最后一个被选中的词语.
 * @return 执行状况
 */
bool PinyinEditor::RevokeSelectedPhrase() {
  /* 移除最后被选中的词语 */
  if (accepted_phrase_list_.empty())
    return false;
  delete accepted_phrase_list_.back();
  accepted_phrase_list_.pop_back();
  /* 清空必要缓冲数据 */
  ClearCachePhraseList();
  ClearPhraseStorageList();
  /* 查询词语代理 */
  LookupPhraseProxy();

  return true;
}

/**
 * 获取原始串.
 * @retval text 原始串
 * @retval len 原始串长度
 * @return 执行状况
 */
void PinyinEditor::GetRawText(char **text, int *len) {
  if (pinyin_table_.empty()) {
    *len = 0;
    *text = NULL;
    return;
  }

  *len = pinyin_table_.size();
  *text = (char *)malloc(*len);
  memcpy(*text, pinyin_table_.c_str(), *len);
}

/**
 * 获取提交数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 */
void PinyinEditor::GetCommitText(char **text, int *len) {
  if (accepted_phrase_list_.empty()) {
    *len = 0;
    *text = NULL;
    return;
  }

  /* 获取数据总长度 */
  size_t length = 0;
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    length += phrase_datum->raw_data_length_;
  }

  /* 拷贝数据 */
  *len = 0;
  *text = (char *)malloc(length);
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    memcpy(*text + *len, phrase_datum->raw_data_,
           phrase_datum->raw_data_length_);
    *len += phrase_datum->raw_data_length_;
  }
}

/**
 * 获取预编辑数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 */
void PinyinEditor::GetPreeditText(char **text, int *len) {
  if (accepted_phrase_list_.empty() && cache_phrase_list_.empty()) {
    *len = 0;
    *text = NULL;
    return;
  }

  /* 获取数据总长度 */
  size_t length = 0;
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    length += phrase_datum->raw_data_length_;
  }
  if (!cache_phrase_list_.empty()) {
    PhraseDatum *phrase_datum = cache_phrase_list_.front();
    length += phrase_datum->raw_data_length_ + 1;
  }

  /* 拷贝数据 */
  *len = 0;
  *text = (char *)malloc(length);
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    memcpy(*text + *len, phrase_datum->raw_data_,
           phrase_datum->raw_data_length_);
    *len += phrase_datum->raw_data_length_;
  }
  if (!cache_phrase_list_.empty()) {
    if (*len != 0) {
      *(*text + *len) = '\x20';
      ++(*len);
    }
    PhraseDatum *phrase_datum = cache_phrase_list_.front();
    memcpy(*text + *len, phrase_datum->raw_data_,
           phrase_datum->raw_data_length_);
    *len += phrase_datum->raw_data_length_;
  }
}

/**
 * 获取辅助数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 */
void PinyinEditor::GetAuxiliaryText(char **text, int *len) {
  if (accepted_phrase_list_.empty() && !chars_proxy_) {
    *len = 0;
    *text = NULL;
    return;
  }

  /* 获取数据总长度 */
  int chars_proxy_offset = 0;
  size_t length = 0;
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    chars_proxy_offset += phrase_datum->chars_proxy_length_;
    length += phrase_datum->raw_data_length_;
  }
  char *pinyin = NULL;
  size_t pinyin_length = 0;
  if (chars_proxy_offset != chars_proxy_length_) {
    PinyinParser pinyin_parser;
    pinyin = pinyin_parser.UnparsePinyin(
                 chars_proxy_ + chars_proxy_offset,
                 chars_proxy_length_ - chars_proxy_offset);
    pinyin_length = strlen(pinyin);
    length += pinyin_length + 1;
  }

  /* 拷贝数据 */
  *len = 0;
  *text = (char *)malloc(length);
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    memcpy(*text + *len, phrase_datum->raw_data_,
           phrase_datum->raw_data_length_);
    *len += phrase_datum->raw_data_length_;
  }
  if (pinyin) {
    if (*len != 0) {
      *(*text + *len) = '\x20';
      ++(*len);
    }
    memcpy(*text + *len, pinyin, pinyin_length);
    *len += pinyin_length;
    free(pinyin);  // 必须释放
  }
}

/**
 * 获取一个页面的词语数据.
 * @param pagesize 页面大小
 * @retval list 词语链表
 */
void PinyinEditor::GetPagePhrase(int pagesize,
                                 std::list<const PhraseDatum *> *list) {
  int count = 0;
  while (count < pagesize) {
    PhraseProxyStorage *storage = SearchPreferPhrase();
    if (!storage)
      break;
    PhraseProxy *phrase_proxy = storage->phrase_proxy_list_->front();
    storage->phrase_proxy_list_->pop_front();
    PhraseDatum *phrase_datum =
        storage->phrase_proxy_site_->phrase_->AnalyzePhraseProxy(phrase_proxy);
    if (IsExistCachePhrase(phrase_datum)) {
      delete phrase_datum;
    } else {
      list->push_back(phrase_datum);
      cache_phrase_list_.push_back(phrase_datum);
      ++count;
    }
    delete phrase_proxy;
  }
}

/**
 * 获取动态词语.
 * @retval list 词语链表
 */
void PinyinEditor::GetDynamicPhrase(std::list<const PhraseDatum *> *list) {
  std::list<PhraseDatum *> phrase_datum_list;
  DynamicPhrase *dynamic_phrase = DynamicPhrase::GetInstance();
  dynamic_phrase->GetDynamicPhrase(pinyin_table_.c_str(), &phrase_datum_list);

  for (std::list<PhraseDatum *>::iterator iterator = phrase_datum_list.begin();
       iterator != phrase_datum_list.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    list->push_back(phrase_datum);
    cache_phrase_list_.push_back(phrase_datum);
  }
}

/**
 * 获取引擎合成词语.
 * @return 词语数据
 */
const PhraseDatum *PinyinEditor::GetComposePhrase() {
  std::list<PhraseDatum *> phrase_datum_list;
  PhraseDatum *local_phrase_datum = NULL;

  /* 获取合成词语的各部分词语数据 */
  /*/* 第一个词语数据 */
  if (cache_phrase_list_.empty()) {
    PhraseProxyStorage *storage = SearchPreferPhrase();
    if (storage) {
      PhraseProxy *phrase_proxy = storage->phrase_proxy_list_->front();
      local_phrase_datum = storage->phrase_proxy_site_->phrase_
                               ->AnalyzePhraseProxy(phrase_proxy);
    }
  } else {
    local_phrase_datum = cache_phrase_list_.front();
  }
  if (!local_phrase_datum)
    return NULL;
  phrase_datum_list.push_back(local_phrase_datum);
  int offset = FinishCharsOffset() + local_phrase_datum->chars_proxy_length_;
  /*/* 其余词语数据 */
  while (offset != chars_proxy_length_) {
    PhraseProxyStorage *storage =
        phrase_manager_->SearchPreferPhrase(chars_proxy_ + offset,
                                            chars_proxy_length_ - offset);
    if (!storage)
      break;
    PhraseProxy *phrase_proxy = storage->phrase_proxy_list_->front();
    local_phrase_datum =
        storage->phrase_proxy_site_->phrase_->AnalyzePhraseProxy(phrase_proxy);
    phrase_datum_list.push_back(local_phrase_datum);
    offset += local_phrase_datum->chars_proxy_length_;
    delete storage;
  }
  if (phrase_datum_list.size() == 1) {
    if (cache_phrase_list_.empty())
      delete phrase_datum_list.front();
    return NULL;
  }

  /* 计算需要的空间 */
  int chars_proxy_length = 0;
  size_t length = 0;
  for (std::list<PhraseDatum *>::iterator iterator = phrase_datum_list.begin();
       iterator != phrase_datum_list.end();
       ++iterator) {
    local_phrase_datum = *iterator;
    chars_proxy_length += local_phrase_datum->chars_proxy_length_;
    length += local_phrase_datum->raw_data_length_;
  }

  /* 组装词语 */
  PhraseDatum *phrase_datum = new PhraseDatum;
  phrase_datum->chars_proxy_ = new CharsProxy[chars_proxy_length];
  phrase_datum->raw_data_ = malloc(length);
  phrase_datum->phrase_data_offset_ = EnginePhraseType;
  for (std::list<PhraseDatum *>::iterator iterator = phrase_datum_list.begin();
       iterator != phrase_datum_list.end();
       ++iterator) {
    local_phrase_datum = *iterator;
    memcpy(phrase_datum->chars_proxy_ + phrase_datum->chars_proxy_length_,
           local_phrase_datum->chars_proxy_,
           sizeof(CharsProxy) * local_phrase_datum->chars_proxy_length_);
    phrase_datum->chars_proxy_length_ +=
        local_phrase_datum->chars_proxy_length_;
    memcpy((char *)phrase_datum->raw_data_ + phrase_datum->raw_data_length_,
           local_phrase_datum->raw_data_,
           local_phrase_datum->raw_data_length_);
    phrase_datum->raw_data_length_ += local_phrase_datum->raw_data_length_;
  }

  /* 善后工作 */
  if (cache_phrase_list_.empty())
    delete phrase_datum_list.front();
  phrase_datum_list.pop_front();
  STL_DELETE_DATA(phrase_datum_list, std::list<PhraseDatum *>);

  /* 加入缓冲链表 */
  cache_phrase_list_.push_back(phrase_datum);

  return phrase_datum;
}

/**
 * 选定词语数据缓冲区中的词语数据.
 * @param datum 词语数据
 */
void PinyinEditor::SelectCachePhrase(const PhraseDatum *datum) {
  /* 将词语数据加入已接受词语链表 */
  std::list<PhraseDatum *>::iterator iterator =
      find(cache_phrase_list_.begin(), cache_phrase_list_.end(), datum);
  if (iterator == cache_phrase_list_.end())
    return;
  accepted_phrase_list_.push_back(*iterator);
  cache_phrase_list_.erase(iterator);
  /* 清空缓冲数据 */
  ClearCachePhraseList();
  ClearPhraseStorageList();
  /* 如果需要则继续查询词语 */
  if (!IsFinishTask())
    LookupPhraseProxy();
}

/**
 * 删除词语数据.
 * @param datum 词语数据
 */
void PinyinEditor::DeletePhraseData(const PhraseDatum *datum) {
  std::list<PhraseDatum *>::iterator iterator =
      find(cache_phrase_list_.begin(), cache_phrase_list_.end(), datum);
  if (iterator == cache_phrase_list_.end())
    return;
  phrase_manager_->DeletePhraseDatum(datum);
  delete *iterator;
  cache_phrase_list_.erase(iterator);
}

/**
 * 反馈被用户选中的词语.
 */
void PinyinEditor::FeedbackSelectedPhrase() {
  /* 若没有数据则无需处理 */
  size_t size = accepted_phrase_list_.size();
  if (size == 0)
    return;
  /* 获取需要被反馈的词语数据 */
  PhraseDatum *phrase_datum = NULL;
  if (size > 1)
    phrase_datum = CreateUserPhrase();
  else
    phrase_datum = accepted_phrase_list_.front();
  /* 反馈词语数据 */
  phrase_manager_->FeedbackPhraseDatum(phrase_datum);
  /* 如果词语是临时合成，则需要手工释放 */
  if (size > 1)
    delete phrase_datum;
}

/**
 * 词语查询工作是否已经完成.
 * @return 是否完成
 */
bool PinyinEditor::IsFinishTask() {
  return FinishCharsOffset() == chars_proxy_length_;
}

/**
 * 结束本次任务.
 */
void PinyinEditor::StopTask() {
  Clear();
}

/**
 * 获取即将被用来反馈的词汇的偏移量.
 * @return 词汇偏移量
 * @note 请不要将此偏移量用作其他用途，它只应该被用来判断词汇所属的类型.
 */
int PinyinEditor::GetPhraseOffset() {
  size_t size = accepted_phrase_list_.size();
  if (size > 1)
    return ManualPhraseType;
  else if (size == 1)
    return accepted_phrase_list_.front()->phrase_data_offset_;
  else
    return InvalidPhraseType;
}

/**
 * 创建用户词语.
 * @return 词语数据
 */
PhraseDatum *PinyinEditor::CreateUserPhrase() {
  if (accepted_phrase_list_.empty())
    return NULL;

  /* 计算需要的内存长度 */
  int chars_proxy_length = 0;
  size_t length = 0;
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    chars_proxy_length += phrase_datum->chars_proxy_length_;
    length += phrase_datum->raw_data_length_;
  }

  /* 创建新的词语数据 */
  PhraseDatum *phrase_datum = new PhraseDatum;
  phrase_datum->chars_proxy_ = new CharsProxy[chars_proxy_length];
  phrase_datum->raw_data_ = malloc(length);
  phrase_datum->phrase_data_offset_ = ManualPhraseType;
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *local_phrase_datum = *iterator;
    memcpy(phrase_datum->chars_proxy_ + phrase_datum->chars_proxy_length_,
           local_phrase_datum->chars_proxy_,
           sizeof(CharsProxy) * local_phrase_datum->chars_proxy_length_);
    phrase_datum->chars_proxy_length_ +=
        local_phrase_datum->chars_proxy_length_;
    memcpy((char *)phrase_datum->raw_data_ + phrase_datum->raw_data_length_,
           local_phrase_datum->raw_data_,
           local_phrase_datum->raw_data_length_);
    phrase_datum->raw_data_length_ += local_phrase_datum->raw_data_length_;
  }

  return phrase_datum;
}

/**
 * 搜索各个词语集合所提供的最佳词语.
 * @return 词语缓冲点
 */
PhraseProxyStorage *PinyinEditor::SearchPreferPhrase() {
  if (!phrase_storage_list_)
    return NULL;

  PhraseProxyStorage *storage = NULL;
  int chars_proxy_length = 0;
  int priority = 0;
  for (std::list<PhraseProxyStorage *>::iterator iterator =
           phrase_storage_list_->begin();
       iterator != phrase_storage_list_->end();
       ++iterator) {
    PhraseProxyStorage *local_storage = *iterator;
    if (local_storage->phrase_proxy_list_->empty())
      continue;
    int local_chars_proxy_length =
        local_storage->phrase_proxy_list_->front()->chars_proxy_length_;
    int loca_priority = local_storage->phrase_proxy_site_->priority_;
    if (!storage ||
        chars_proxy_length < local_chars_proxy_length ||
        (chars_proxy_length == local_chars_proxy_length &&
         priority < loca_priority)) {
      storage = local_storage;
      chars_proxy_length = local_chars_proxy_length;
      priority = loca_priority;
    }
  }
  return storage;
}

/**
 * 创建汉字代理数组.
 */
void PinyinEditor::CreateCharsProxy() {
  /* 如果处于英文模式，则直接退出 */
  if (!editor_mode_)
    return;

  /* 创建汉字代理数组 */
  const std::list<OuterMendPinyinPair *> *mend_pair_table =
      phrase_manager_->GetMendPinyinTable();
  char *pinyin1 = AmendPinyinString(pinyin_table_.c_str(), mend_pair_table);
  char *pinyin2 = AmendPinyinString(pinyin1, mend_pair_table_);
  PinyinParser pinyin_parser;
  pinyin_parser.ParsePinyin(pinyin2, &chars_proxy_, &chars_proxy_length_);
  free(pinyin1);
  free(pinyin2);
}

/**
 * 查询词语代理.
 */
void PinyinEditor::LookupPhraseProxy() {
  /* 如果处于英文模式，则直接退出 */
  if (!editor_mode_)
    return;

  /* 查询词语代理 */
  int offset = FinishCharsOffset();
  phrase_storage_list_ = phrase_manager_->SearchMatchablePhrase(
                                              chars_proxy_ + offset,
                                              chars_proxy_length_ - offset);
}

/**
 * 计算当前待查询汉字代理数组的偏移量.
 * @return 偏移量
 */
int PinyinEditor::FinishCharsOffset() {
  int length = 0;
  for (std::list<PhraseDatum *>::iterator iterator =
           accepted_phrase_list_.begin();
       iterator != accepted_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    length += phrase_datum->chars_proxy_length_;
  }
  return length;
}

/**
 * 考察此词语是否已经存在词语数据缓冲区中.
 * @param datum 词语数据
 * @return 是否已经存在
 */
bool PinyinEditor::IsExistCachePhrase(const PhraseDatum *datum) {
  bool result = false;
  for (std::list<PhraseDatum *>::iterator iterator = cache_phrase_list_.begin();
       iterator != cache_phrase_list_.end();
       ++iterator) {
    PhraseDatum *phrase_datum = *iterator;
    if (datum->raw_data_length_ == phrase_datum->raw_data_length_ &&
        memcmp(datum->raw_data_, phrase_datum->raw_data_,
               datum->raw_data_length_) == 0) {
      result = true;
      break;
    }
  }
  return result;
}

/**
 * 纠正拼音串中可能存在的错误.
 * 使用(PhraseManager)类提供的拼音修正表. \n
 * @param string 原拼音串
 * @param mend_pair_table 拼音修正参考表
 * @return 新拼音串
 */
char *PinyinEditor::AmendPinyinString(
          const char *string,
          const std::list<OuterMendPinyinPair *> *mend_pair_table) {
  std::string mend_string;
  size_t length = strlen(string);
  mend_string.reserve(length << 1);

  size_t count = 0;
  while (count < length) {
    std::list<OuterMendPinyinPair *>::const_iterator iterator =
        mend_pair_table->begin();
    const char *raw = NULL;
    for (; iterator != mend_pair_table->end(); ++iterator) {
      raw = (*iterator)->raw_;
      if (strncmp(string + count, raw, strlen(raw)) == 0)
        break;
    }
    if (iterator != mend_pair_table->end()) {
      mend_string.append((*iterator)->mend_);
      count += strlen(raw);
    } else {
      mend_string.append(1, *(string + count));
      ++count;
    }
  }

  return strdup(mend_string.c_str());
}

/**
 * 纠正拼音串中可能存在的错误.
 * 使用本类内置的拼音修正表. \n
 * @param string 原拼音串
 * @param mend_pair_table 拼音修正参考表
 * @return 新拼音串
 */
char *PinyinEditor::AmendPinyinString(
          const char *string,
          const InnerMendPinyinPair *mend_pair_table) {
  std::string mend_string;
  size_t length = strlen(string);
  mend_string.reserve(length << 1);

  size_t count = 0;
  while (count < length) {
    const InnerMendPinyinPair *mend_pinyin_pair = mend_pair_table;
    for (; mend_pinyin_pair->raw; ++mend_pinyin_pair) {
      if (strncmp(string + count, mend_pinyin_pair->raw,
                  strlen(mend_pinyin_pair->raw)) == 0)
        break;
    }
    if (mend_pinyin_pair->raw) {
      mend_string.append(mend_pinyin_pair->mend);
      count += strlen(mend_pinyin_pair->raw);
    } else {
      mend_string.append(1, *(string + count));
      ++count;
    }
  }

  return strdup(mend_string.c_str());
}

/**
 * 清除本编辑器的所有缓冲数据.
 */
void PinyinEditor::Clear() {
  cursor_point_ = 0;
  pinyin_table_.clear();

  ClearCharsProxy();
  ClearAcceptedPhraseList();
  ClearCachePhraseList();
  ClearPhraseStorageList();
}

/**
 * 清除汉字代理数组.
 */
void PinyinEditor::ClearCharsProxy() {
  delete [] chars_proxy_;
  chars_proxy_ = NULL;
  chars_proxy_length_ = 0;
}

/**
 * 清除已接受的词语数据.
 */
void PinyinEditor::ClearAcceptedPhraseList() {
  STL_DELETE_DATA(accepted_phrase_list_, std::list<PhraseDatum *>);
  accepted_phrase_list_.clear();
}

/**
 * 清除缓冲的词语数据.
 */
void PinyinEditor::ClearCachePhraseList() {
  STL_DELETE_DATA(cache_phrase_list_, std::list<PhraseDatum *>);
  cache_phrase_list_.clear();
}

/**
 * 清除储存点数据.
 */
void PinyinEditor::ClearPhraseStorageList() {
  if (!phrase_storage_list_)
    return;

  STL_DELETE_DATA(*phrase_storage_list_, std::list<PhraseProxyStorage *>);
  delete phrase_storage_list_;
  phrase_storage_list_ = NULL;
}
