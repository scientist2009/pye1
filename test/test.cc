/***************************************************************************
 *   Copyright (C) 2009, 2010 by Jally   *
 *   jallyx@163.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "engine/dynamic_phrase.h"
#include "engine/phrase_manager.h"
#include "engine/pinyin_editor.h"

void PrintData(void *data, int len) {
  char buffer[4096];
  memcpy(buffer, data, len);
  strcpy(buffer + len, "\x20");
  printf("%s", buffer);
}

int main(int argc, char *argv[]) {
  PhraseManager *phrase_manager = PhraseManager::GetInstance();
  phrase_manager->CreateSystemPhraseProxySite("config.txt");
  phrase_manager->CreateUserPhraseProxySite("user.mb");

  PinyinEditor pinyin_editor(phrase_manager);
  std::list<const PhraseDatum *> global_list;
  const int pagesize = 5;
  int pages(0), page(0);

  char ch = '\0';
  while ((ch = getchar()) != '`') {
    if (ch >= 'a' && ch <= 'z') {
      pinyin_editor.InsertPinyinKey(ch);
mark1:
      global_list.clear();
      pages = page = 0;

      std::list<const PhraseDatum *> list;
      pinyin_editor.GetPagePhrase(pagesize, &list);
      int number = 1;
      for (std::list<const PhraseDatum *>::iterator iterator = list.begin();
           iterator != list.end();
           ++iterator) {
        const PhraseDatum *datum = *iterator;
        printf("%d.", number);
        PrintData(datum->raw_data_, datum->raw_data_length_);
        printf("\x20");
        ++number;

        global_list.push_back(datum);
      }
      if (!list.empty()) {
        pages = page = 1;
        printf("\n");
      }

      char *data = NULL;
      int len = 0;
      pinyin_editor.GetRawText(&data, &len);
      if (data) {
        printf("RawText: ");
        PrintData(data, len);
        printf("\n");
        free(data);
      }
      pinyin_editor.GetPreeditText(&data, &len);
      if (data) {
        printf("PreeditText: ");
        PrintData(data, len);
        printf("\n");
        free(data);
      }
      pinyin_editor.GetAuxiliaryText(&data, &len);
      if (data) {
        printf("AuxiliaryText: ");
        PrintData(data, len);
        printf("\n");
        free(data);
      }

      const PhraseDatum *datum = pinyin_editor.GetComposePhrase();
      if (datum) {
        printf("ComposePhrase: ");
        PrintData(datum->raw_data_, datum->raw_data_length_);
        printf("\n");
      }
    } else if (ch >= '1' && ch <= '5') {
      int index = pagesize * (page - 1) + (ch - '1');
      if ((size_t)index >= global_list.size())
        continue;
      std::list<const PhraseDatum *>::iterator iterator = global_list.begin();
      advance(iterator, index);
      const PhraseDatum *datum = *iterator;
      printf("SelectedPhrase: ");
      PrintData(datum->raw_data_, datum->raw_data_length_);
      printf("\n");

      char *data = NULL;
      int len = 0;
      pinyin_editor.SelectCachePhrase(datum);
      if (pinyin_editor.IsFinishTask()) {
        pinyin_editor.GetCommitText(&data, &len);
        printf("CommitText: ");
        PrintData(data, len);
        printf("\n");
        free(data);

        pinyin_editor.FeedbackSelectedPhrase();
        phrase_manager->BackupUserPhrase();

        pinyin_editor.StopTask();
        global_list.clear();
        pages = page = 0;
      } else {
        goto mark1;
      }
    } else if (ch == '=') {
      if (pages == page) {
        std::list<const PhraseDatum *> list;
        pinyin_editor.GetPagePhrase(pagesize, &list);
        int number = 1;
        for (std::list<const PhraseDatum *>::iterator iterator = list.begin();
             iterator != list.end();
             ++iterator) {
          const PhraseDatum *datum = *iterator;
          printf("%d.", number);
          PrintData(datum->raw_data_, datum->raw_data_length_);
          printf("\x20");
          ++number;

          global_list.push_back(datum);
        }
        if (!list.empty()) {
          ++pages;
          ++page;
          printf("\n");
        }
      } else {
        ++page;
mark2:
        int index = pagesize * (page - 1);
        std::list<const PhraseDatum *>::iterator iterator = global_list.begin();
        advance(iterator, index);
        for (int number = 1; number <= pagesize; ++number) {
          if (iterator == global_list.end())
            break;
          const PhraseDatum *datum = *iterator;
          printf("%d.", number);
          PrintData(datum->raw_data_, datum->raw_data_length_);
          printf("\x20");
          ++iterator;
        }
        printf("\n");
      }
    } else if (ch == '-') {
      if (page <= 1)
        continue;
      --page;
      goto mark2;
    }
  }

  return 0;
}
