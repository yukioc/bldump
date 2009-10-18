# Makefile of bldump
#
# $Id$
#

#### CONFIGURE
APP_EXE		:= bldump
APP_SRC		:= bldump.c verbose.c
APP_VER		:= $(shell git describe)
TEST_EXE	:= $(APP_EXE)-test
TEST_SRC	:= $(wildcard t-*.c)
TEST_LIB	:= cunit
DOXYGEN_CFG	:= bldump.dox
CUNIT_DIR	:= ./CUnit
CUNIT_VER	:= CUnit-2.1-0
ALL_SRC		:= $(APP_SRC) $(TEST_SRC)

ifdef COMSPEC
APP_EXE		:=$(APP_EXE:%=%.exe)
TEST_EXE	:=$(TEST_EXE:%=%.exe)
endif

VPATH	:= ${CUNIT_DIR}

##### COMMAND
CC			=gcc
CPPFLAGS	=-O3 -DVERSION="\"${APP_VER}\""
SC_INCDIR	=-I.

ifdef COMSPEC
  CFLAGS =-mno-cygwin
else
  CFLAGS =
  #CFLAGS =-g
endif

# CC		= cl
# LXFLAGS	= /nologo /W3
# CPPFLAGS	= /nologo /GX /W3

##### TARGET
.PHONEY: all
all: depend $(APP_EXE)

.PHONEY: clean
clean:
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno
	rm -f $(CUNIT_VER)-src.tar.gz

.PHONEY: test
test: clean depend libcunit.a
	make $(TEST_EXE) CFLAGS="$(CFLAGS) -DCUNIT -ftest-coverage -fprofile-arcs"
	@echo "### cunit"
	./$(TEST_EXE) --test
	@echo "### gcov"
	gcov $(APP_SRC)

PHONEY: depend
depend: .depend

.depend : $(SRC_ALL)
	@echo "### depend"
	${CXX} ${CFLAGS} ${SC_INCDIR} -MM $^ > $@

include .depend

$(APP_EXE) : $(APP_SRC:%.c=%.o) 
	@echo "### $@ ###"
	${CC} ${CFLAGS} ${SC_INCDIR} -o $@ $^

$(TEST_EXE) : $(APP_SRC:%.c=%.o) $(TEST_SRC:%.c=%.o)
	@echo "### $@ ###"
	${CC} ${CFLAGS} ${SC_INCDIR} -L${CUNIT_DIR} -o $@ $^ -lcunit

libcunit.a :
	@echo "### $@ ###"
	wget http://sourceforge.net/projects/cunit/files/CUnit/2.1-0/$(CUNIT_VER)-src.tar.gz/download
	tar xfz $(CUNIT_VER)-src.tar.gz
	cd $(CUNIT_VER) && sh ./configure && $(MAKE) MAKEFLAGS=
	cp -r $(CUNIT_VER)/CUnit/Headers ${CUNIT_DIR}
	cp $(CUNIT_VER)/CUnit/Sources/.libs/libcunit.a  ${CUNIT_DIR}/
	rm $(CUNIT_VER)-src.tar.gz
	rm -r $(CUNIT_VER)

.PHONEY: doxygen
doxygen:
	@echo "### doxygen"
	doxygen $(DOXYGEN_CFG)

.PHONEY: splint
splint:
	@echo "### splint"
#	splint -standard 
	splint -weak \
		-duplicatequals \
		-mustfreeonly \
		-observertrans -statictrans -nullstate \
		-compdestroy \
		-sefparams \
		-unqualifiedtrans -branchstate -globstate \
		+longunsignedintegral \
		-boolcompare \
		-shiftnegative \
		-formatconst \
		-shiftimplementation \
		-unrecog \
		-dependenttrans \
		-I. -DBLDUMP_DEBUG \
		$(ALL_SRC)

##### SUFFIX
.SUFFIXES: .c .exe

.o.exe:
	@echo "### $@ ###"
	${CC} ${CFLAGS} ${SC_INCDIR} -o $@ $^

.c.o:
	@echo "### $@ ###"
	${CC} ${CFLAGS} ${SC_INCDIR} ${CPPFLAGS} -o $@ -x c -c $< 

