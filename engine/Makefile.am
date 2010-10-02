METASOURCES = AUTO
lib_LTLIBRARIES = libpye.la

libpye_la_SOURCES = dynamic_phrase.cc phrase_manager.cc pinyin_editor.cc \
                    pinyin_parser.cc system_phrase.cc user_phrase.cc \
                    wrapper.cc

AM_CXXFLAGS = -Wall

# set the include path found by configure

# the library search path.

pyeincludedir=$(includedir)/pye-0.2
pyeinclude_HEADERS = abstract_phrase.h dynamic_phrase.h global.h output.h \
                     phrase_manager.h pinyin_editor.h pinyin_parser.h \
                     system_phrase.h user_phrase.h wrapper.h
