CC = clang
CXX = clang++

ifneq ($(wildcard src/*.cpp),)
	CCLD = $(CXX)
else
	CCLD = $(CC)
endif

#PKGCONFIGS = 
#PKGCONFIGS_CFLAGS = $(shell pkg-config --cflags $(PKGCONFIGS))
#PKGCONFIGS_LDFLAGS = $(shell pkg-config --libs $(PKGCONFIGS))

CFLAGS = $(shell pkg-config --cflags apr-1) -Os -Wall -ggdb
CXXFLAGS = $(CFLAGS) -std=c++11
LDFLAGS = -Llib -lapr-1 \
	-static-libgcc -Wl,--as-needed,--gc-sections

ALL_CFLAGS = $(PKGCONFIGS_CFLAGS) $(CFLAGS)
ALL_CXXFLAGs = $(PKGCONFIGS_CFLAGS) $(CXXFLAGS)
ALL_LDFLAGS = $(PKGCONFIGS_LDFLAGS) $(LDFLAGS)

SRCFILES = $(wildcard src/*.c src/*.cpp)
OBJFILES = $(patsubst src/%,bin/%.o,$(SRCFILES))
DEPFILES = $(patsubst src/%,dep/%.d,$(SRCFILES))

TARGET = coropx

IDEPFILES = $(DEPFILES)
ifeq ($(MAKECMDGOALS),clean)
	IDEPFILES =
endif

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f dep/* bin/* $(TARGET)

dep/%.c.d: src/%.c
	$(CC) -MM -MT $(patsubst src/%,bin/%.o,$<) -MT $@ -MF $@ $< $(ALL_CFLAGS)

dep/%.cpp.d: src/%.cpp
	$(CXX) -MM -MT $(patsubst src/%,bin/%.o,$<) -MT $@ -MF $@ $< $(ALL_CXXFLAGS)

bin/%.c.o: src/%.c
	$(CC) $< -c -o $@ $(ALL_CFLAGS)

bin/%.cpp.o: src/%.cpp
	$(CXX) $< -c -o $@ $(ALL_CXXFLAGS)

$(TARGET): $(OBJFILES)
	$(CCLD) $^ -o $@ $(ALL_LDFLAGS)

-include $(IDEPFILES)
