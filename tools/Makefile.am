bin_PROGRAMS = pye-create-mb pye-create-umb pye-parse-umb

pye_create_mb_SOURCES = mb_creater.cc pye_create_mb.cc
pye_create_umb_SOURCES = pye_create_umb.cc umb_creater.cc
pye_parse_umb_SOURCES = pye_parse_umb.cc umb_parser.cc

AM_CPPFLAGS = -I$(top_srcdir)
AM_CXXFLAGS = -Wall

LIBS = $(top_builddir)/engine/libpye.la

noinst_HEADERS = mb_creater.h umb_creater.h umb_parser.h
