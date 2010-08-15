# Makefile of bldump

TEST:=0

#### FILE
APP_EXE		:= bldump
APP_SRC		:= bldump.c verbose.c
APP_VER		:= $(shell git describe)
TEST_EXE	:= $(APP_EXE)-test
TEST_SRC	:= $(wildcard t-*.c)
DOXYGEN_CFG	:= bldump.dox
ALL_SRC		:= $(APP_SRC) $(TEST_SRC)

##### PATH
#VPATH	:= 

##### COMMAND
CC			:=gcc

CFLAGS		:=-O3
#CFLAGS		+=-g
CPPFLAGS	+=-Wall -Wextra
INCLUDES	:=-I.

##### OPTIONAL
ifdef COMSPEC
APP_EXE		:=$(APP_EXE:%=%.exe)
TEST_EXE	:=$(TEST_EXE:%=%.exe)
# CC		= cl
CFLAGS		:=-mno-cygwin
# LXFLAGS	= /nologo /W3
# CPPFLAGS	= /nologo /GX /W3
endif
ifdef APP_VER
CPPFLAGS	+=-DVERSION="\"${APP_VER}\""
endif
ifeq ($(TEST),1)
CFLAGS		+=-g
CFLAGS		+=-DTEST -ftest-coverage -fprofile-arcs
endif

##### TARGET
.PHONEY: all
all: depend $(APP_EXE)

.PHONEY: clean
clean:
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno

.PHONEY: test
test: clean depend tp gcov

.PHONEY: tp
tp:
	@echo "### cunit"
	make $(TEST_EXE) TEST=1
	./$(TEST_EXE) --test

.PHONY: gcov
gcov:
	@echo "### gcov"
	@gcov $(APP_SRC) | \
	grep -v creating | \
	perl -pe 's/File .([\S]+).\n/\1 /; s/Lines \w+:(.+)\n/\1/;' | \
	grep -v -E "(^/usr|^t_/)" | \
	perl -ne 'printf "%20s %7s %s %3s\n", split;'

PHONEY: depend
depend: .depend

.depend : $(SRC_ALL)
	@echo "### depend"
	${CXX} ${CFLAGS} ${INCLUDES} -MM $^ > $@

include .depend

$(APP_EXE) : $(APP_SRC:%.c=%.o) 
	@echo "### $@ ###"
	${CC} ${CFLAGS} ${INCLUDES} -o $@ $^

$(TEST_EXE) : $(APP_SRC:%.c=%.o) $(TEST_SRC:%.c=%.o)
	@echo "### $@ ###"
	${CC} ${CFLAGS} ${INCLUDES} -o $@ $^

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
	@${CC} ${CFLAGS} ${INCLUDES} -o $@ $^

.c.o:
	@echo "### $@ ###"
	@${CC} ${CFLAGS} ${INCLUDES} ${CPPFLAGS} -o $@ -x c -c $< 

