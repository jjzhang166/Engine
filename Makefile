### Settings
ifeq ($(OS), Windows_NT)
	CFLAGS	= -g -O2 -pipe
	LIBRARY	= libengine.dll
	OUTPUT	= -o $(LIBRARY) -Wl,--out-implib,$(LIBRARY).a
	LDFLAGS	= -lpthread -Llib -lluajit_mingw -lws2_32 -lkernel32 -luserenv -luser32 -lgdi32 -lshell32 -lwinspool -lole32
else
	CFLAGS	= -fPIC -O2 -pipe
	LIBRARY	= libengine.so
	OUTPUT	= -o $(LIBRARY) -Wl,-soname,$(LIBRARY)
	LDFLAGS = -lpthread -Llib -lluajit -luuid -lrt -ldl
endif

CC			= gcc
CXX			= g++
CXXFLAGS	= $(CFLAGS) -std=c++11 -pthread
INCS		= -I./include -I./3rdpart/luajit/include

SRCDIR		= src src/Miniz

### Auto-Generate
SRCS	= $(foreach path, $(SRCDIR), $(wildcard $(path)/*.cc))
OBJS	= $(patsubst %.cc, %.o, $(SRCS))

### Targets
library: prebuild build

prebuild:
	@echo -e "\033[36m=== Start to build shared library libengine ===\033[0m"

build: $(OBJS)
	@echo -e "\033[36m>>> Generate shared library libengine ...\033[0m"
	@$(CXX) -fPIC -shared $(OUTPUT) $(OBJS) $(LDFLAGS)

clean:
	@rm -f $(OBJS) $(LIBRARY)

### Rules
%.o: %.cc
	@echo -e "\033[33m---> Compile $< ...\033[0m"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCS)
