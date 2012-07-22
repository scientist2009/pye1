lib_LTLIBRARIES = libpye.la

libpye_la_SOURCES = dynamic_phrase.cc phrase_manager.cc pinyin_editor.cc \
                    pinyin_parser.cc pye_wrapper.cc system_phrase.cc \
                    user_phrase.cc

AM_CPPFLAGS = -I$(top_srcdir)
AM_CXXFLAGS = -Wall

pyeincludedir=$(includedir)/pye-0.2
pyeinclude_HEADERS = abstract_phrase.h dynamic_phrase.h phrase_manager.h \
                     pinyin_editor.h pinyin_parser.h pye_global.h pye_output.h \
                     pye_wrapper.h system_phrase.h user_phrase.h
