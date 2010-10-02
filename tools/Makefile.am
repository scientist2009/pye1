METASOURCES = AUTO
bin_PROGRAMS = pye-create-mb pye-create-umb pye-parse-umb

pye_create_mb_SOURCES = mb_creater.cc pye_create_mb.cc
pye_create_umb_SOURCES = pye_create_umb.cc umb_creater.cc
pye_parse_umb_SOURCES = pye_parse_umb.cc umb_parser.cc

AM_CXXFLAGS = -Wall

# set the include path found by configure

# the library search path.
LIBS = ../engine/libpye.la

noinst_HEADERS = mb_creater.h umb_creater.h umb_parser.h
