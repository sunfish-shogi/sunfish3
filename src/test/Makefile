GPP:=g++

override CFLAGS+=-std=c++0x
override CFLAGS+=-DSUNFISH_TEST_SAMPLE

all: sample

sample: sample.cpp Test.h ../logger/Logger.h ../logger/Logger.cpp
	$(GPP) -o $@ $(CFLAGS) $^
	
clean:
	rm sample
