### Settings
CC			= gcc
CXX			= g++
CFLAGS		= -fPIC -O2 -pipe
CXXFLAGS	= $(CFLAGS) -std=c++11 -pthread
INCS		= -I./include
LDFLAGS		= -Llib -lluajit -lpthread -luuid -lrt -ldl 

LIBRARY		= libengine.so
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
	@$(CXX) -fPIC -shared -o $(LIBRARY) -Wl,-soname,$(LIBRARY) $(OBJS) $(LDFLAGS)

clean:
	@rm -f $(OBJS) $(LIBRARY)

### Rules
%.o: %.cc
	@echo -e "\033[33m---> Compile $< ...\033[0m"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCS)
