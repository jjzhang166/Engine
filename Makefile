### Settings
CC			= gcc
CXX			= g++
CFLAGS		= -fPIC -O2 -pipe
CXXFLAGS	= $(CFLAGS) -std=c++11
INCS		= -I./include -I./include/lua
LDFLAGS		= -lpthread -luuid -lrt -ldl

LIBRARY		= libengine.so
SRCDIR		= src src/miniz src/lua src/hiredis src/libev

### Auto-Generate
SRCS	= $(foreach path, $(SRCDIR), $(wildcard $(path)/*.cc) $(wildcard $(path)/*.c))
OBJS	= $(patsubst %.c, %.o, $(patsubst %.cc, %.o, $(SRCS)))

### Targets
library: prebuild build

prebuild:
	@echo "\033[36m=== Start to build shared library libengine ===\033[0m"

build: $(OBJS)
	@echo "\033[36m>>> Generate shared library libengine ...\033[0m"
	@$(CXX) -fPIC -shared -o $(LIBRARY) -Wl,-soname,$(LIBRARY) $(OBJS) $(LDFLAGS)

clean:
	@rm -f $(OBJS) $(LIBRARY)

### Rules
%.o: %.cc
	@echo "\033[33m---> Compile $< ...\033[0m"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCS)

%.o: %.c
	@echo "\033[33m---> Compile $< ...\033[0m"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INCS)
