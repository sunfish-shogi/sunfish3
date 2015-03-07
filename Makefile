RM:=rm

SUNFISH:=sunfish
SOURCES:=$(shell find . -name "*.cpp")
OBJECTS:=$(SOURCES:.cpp=.o)
DEPENDS:=$(SOURCES:.cpp=.d)

OPT:=-std=c++11 -DUNIX -Wall -W -msse2 -fno-rtti -pthread -I .
RELEASE_OPT:=-O2 -DNDEBUG -DNLEARN
DEBUG_OPT:=-g -DNLEARN
PROFILE_OPT:=-pg -O2 -DNDEBUG -DNLEARN
LEARN_OPT:=-O2 -DNDEBUG

override CXXFLAGS+=$(OPT)

.PHONY: release release-pgo debug profile learn clean

help:
	@echo 'usage:'
	@echo '  make release'
	@echo '  make release-pgo'
	@echo '  make debug'
	@echo '  make profile'
	@echo '  make learn'
	@echo '  make clean'

release:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(RELEASE_OPT)' $(SUNFISH)

release-pgo:
# TODO

debug:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(DEBUG_OPT)' $(SUNFISH)

profile:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(PROFILE_OPT)' $(SUNFISH)

learn:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(LEARN_OPT)' $(SUNFISH)

$(SUNFISH): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $^

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.d: %.cpp
	@$(SHELL) -c '$(CXX) -MM $(CXXFLAGS) $< | sed "s|^.*:|$*.o $@:|g" > $@; [ -s $@ ] || rm -f $@'

clean:
	$(RM) $(SUNFISH) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)
