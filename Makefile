CMAKE:=cmake
CMAKE_CACHE:=CMakeCache.txt
MKDIR:=mkdir
LN:=ln
TEST:=test

SUNFISH:=sunfish
EVAL_BIN:=eval.bin
BUILD_DIR:=build
PROF:=gprof
PROFOUT:=profile.txt

.PHONY: release release-pgo release-prof debug profile profile1 learn clean run-prof run-prof1

help:
	@echo 'usage:'
	@echo '  make release'
	@echo '  make release-pgo'
	@echo '  make debug'
	@echo '  make test'
	@echo '  make profile'
	@echo '  make profile1'
	@echo '  make learn'
	@echo '  make clean'

release:
	$(MKDIR) -p $(BUILD_DIR)/$@ 2> /dev/null
	cd $(BUILD_DIR)/$@ && \
	$(CMAKE) -D CMAKE_BUILD_TYPE=Release ../../src && \
	$(MAKE)
	$(LN) -s -f $(BUILD_DIR)/$@/$(SUNFISH) $(SUNFISH)

release-pgo:
	$(TEST) -f $(EVAL_BIN)
	$(MKDIR) -p $(BUILD_DIR)/$@ 2> /dev/null
	$(RM) -f $(BUILD_DIR)/$@/$(CMAKE_CACHE)
	cd $(BUILD_DIR)/$@ && \
	$(CMAKE) -D CMAKE_BUILD_TYPE=Release -D PROFILE_GENERATE=ON ../../src && \
	$(MAKE) clean && $(MAKE)
	$(LN) -s -f $(BUILD_DIR)/$@/$(SUNFISH) $(SUNFISH)
	$(MAKE) run-prof
	$(RM) -f $(BUILD_DIR)/$@/$(CMAKE_CACHE)
	cd $(BUILD_DIR)/$@ && \
	$(CMAKE) -D CMAKE_BUILD_TYPE=Release -D PROFILE_USE=ON ../../src && \
	$(MAKE) clean && $(MAKE)

debug:
	$(MKDIR) -p $(BUILD_DIR)/$@ 2> /dev/null
	cd $(BUILD_DIR)/$@ && \
	$(CMAKE) -D CMAKE_BUILD_TYPE=Debug ../../src && \
	$(MAKE)
	$(LN) -s -f $(BUILD_DIR)/$@/$(SUNFISH) $(SUNFISH)

test:
	$(MAKE) debug
	$(SHELL) -c './$(SUNFISH) --test'

release-prof:
	$(TEST) -f $(EVAL_BIN)
	$(MKDIR) -p $(BUILD_DIR)/$@ 2> /dev/null
	cd $(BUILD_DIR)/$@ && \
	$(CMAKE) -D CMAKE_BUILD_TYPE=Release -D PROFILE=ON ../../src && \
	$(MAKE)
	$(LN) -s -f $(BUILD_DIR)/$@/$(SUNFISH) $(SUNFISH)

profile:
	$(MAKE) release-prof
	$(MAKE) run-prof
	$(SHELL) -c '$(PROF) ./$(SUNFISH) > $(PROFOUT)'
	@echo "Look $(PROFOUT)."

profile1:
	$(MAKE) release-prof
	$(MAKE) run-prof1
	$(SHELL) -c '$(PROF) ./$(SUNFISH) > $(PROFOUT)'
	@echo "Look $(PROFOUT)."

learn:
	$(MKDIR) -p $(BUILD_DIR)/$@ 2> /dev/null
	cd $(BUILD_DIR)/$@ && \
	$(CMAKE) -D CMAKE_BUILD_TYPE=Release -D LEARNING=ON ../../src && \
	$(MAKE)
	$(LN) -s -f $(BUILD_DIR)/$@/$(SUNFISH) $(SUNFISH)

clean:
	$(RM) -rf $(BUILD_DIR) $(SUNFISH)

run-prof:
	./$(SUNFISH) --profile -d 30 -t 10

run-prof1:
	./$(SUNFISH) --profile1 -d 30 -t 10
