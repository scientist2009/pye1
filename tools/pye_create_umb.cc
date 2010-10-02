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
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include "umb_creater.h"

const struct option options[] = {
  {"help", 0, NULL, 'h'},
  {"output", 1, NULL, 'o'},
  {"version", 0, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

void PrintUsage() {
  printf("Usage: pye-create-umb inputfile [-o outputfile]\n"
         "\t-o <file> --output=<file>\n\t\tplace the output into <file>\n"
         "\t-h --help\n\t\tdisplay this help and exit\n"
         "\t-v --version\n\t\toutput version information and exit\n");
}

void PrintVersion() {
  printf("pye-create-umb: 0.2.0\n");
}

int main(int argc, char *argv[]) {
  const char *src(NULL), *dst(NULL);
  int opt = -1;
  opterr = 0;
  while ((opt = getopt_long(argc, argv, "ho:v", options, NULL)) != -1) {
    switch (opt) {
      case 'o':
        dst = optarg;
        break;
      case 'h':
        PrintUsage();
        exit(0);
      case 'v':
        PrintVersion();
        exit(0);
      default:
        PrintUsage();
        exit(1);
    }
  }
  if (optind + 1 != argc) {
    PrintUsage();
    exit(1);
  }
  src = argv[optind];
  if (!dst)
    dst = "user.mb";

  UMBCreater umb_creater;
  umb_creater.BuildPhraseTree(src);
  umb_creater.WritePhraseTree(dst);

  return 0;
}
