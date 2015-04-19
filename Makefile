PROF:=gprof
PROFOUT:=profile.txt
RM:=rm

SUNFISH:=sunfish
SOURCES:=$(shell find . -name "*.cpp")
OBJECTS:=$(SOURCES:.cpp=.o)
DEPENDS:=$(SOURCES:.cpp=.d)

OPT:=-std=c++11 -DUNIX -Wall -W -msse2 -fno-rtti -pthread -I .
RELEASE_OPT:=-O3 -DNDEBUG -DNLEARN
DEBUG_OPT:=-g -DNLEARN
PROFILE_OPT:=-pg -O2 -DNDEBUG -DNLEARN
LEARN_OPT:=-O2 -DNDEBUG

override CXXFLAGS+=$(OPT)

.PHONY: release release-pgo debug profile profile1 learn clean run-prof run-prof1

help:
	@echo 'usage:'
	@echo '  make release'
	@echo '  make release-pgo'
	@echo '  make debug'
	@echo '  make profile'
	@echo '  make profile1'
	@echo '  make learn'
	@echo '  make clean'

release:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(RELEASE_OPT)' $(SUNFISH)

release-pgo:
	$(MAKE) clean; $(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(RELEASE_OPT) -fprofile-generate' $(SUNFISH)
	$(MAKE) run-prof
	$(MAKE) clean; $(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(RELEASE_OPT) -fprofile-use' $(SUNFISH)

debug:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(DEBUG_OPT)' $(SUNFISH)

profile:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(PROFILE_OPT)' $(SUNFISH)
	$(MAKE) run-prof
	@$(SHELL) -c '$(PROF) ./$(SUNFISH) > $(PROFOUT)'

profile1:
	$(MAKE) CXXFLAGS='$(CXXFLAGS) $(OPT) $(PROFILE_OPT)' $(SUNFISH)
	$(MAKE) run-prof1
	@$(SHELL) -c '$(PROF) ./$(SUNFISH) > $(PROFOUT)'

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

run-prof:
	@./$(SUNFISH) --profile -d 30 -t 10

run-prof1:
	@./$(SUNFISH) --profile1 -d 30 -t 10

-include $(DEPENDS)
