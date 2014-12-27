GPP:=g++
GCC:=gcc
RM:=rm

PROGRAM:=sunfish
SOURCES:=$(shell find . -name "*.cpp")
OBJECTS:=$(SOURCES:.cpp=.o)
DEPENDS:=$(SOURCES:.cpp=.d)

override CFLAGS+=-std=c++0x
override CFLAGS+=-DPOSIX
override CFLAGS+=-Wall
override CFLAGS+=-W
override CFLAGS+=-O2
#override CFLAGS+=-O3
#override CFLAGS+=-DNDEBUG
override CFLAGS+=-DNLEARN
#override CFLAGS+=-DPRUN_EXPR
override CFLAGS+=-I .
#override CFLAGS+=-g
#override CFLAGS+=-pg
override CFLAGS+=-msse2
override LIBS+=-lrt
override LIBS+=-lboost_program_options
override LIBS+=-lboost_thread
override LIBS+=-lboost_regex

.PHONY: all pgo

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(GPP) -o $(PROGRAM) $(CFLAGS) $^ $(LIBS)

.cpp.o:
	$(GPP) $(CFLAGS) -o $@ -c $<

%.d: %.cpp
	@$(SHELL) -c '$(CC) -MM $(CFLAGS) $< | sed "s|^.*:|$*.o $@:|g" > $@; [ -s $@ ] || rm -f $@'

clean:
	$(RM) $(PROGRAM) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)
