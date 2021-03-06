# # Use 'make V=1' to see the full commands
#使用安静模式可以不看gcc,g++的参数
CC := gcc 
CXX := g++
AR := ar

LIBRARY := libpieke.a
SHARED := libpieke.so

INCLUDE := -I/usr/local/include -I./include -I../rapidmsg/include
LIBS := -L/usr/local/lib -L/usr/lib64/mysql -levent -lpthread -lmysqlclient -lhiredis -lboost_system -lboost_filesystem -lprotobuf

CFLAGS := 
CPPFLAGS := -std=c++11 -O2 -g -fPIC -DNDEBUG
SHARED_LDFLAGS := -shared -fPIC -Wl,-soname,${SHARED}

LIBCFILES := $(wildcard ./util/*.c)
LIBCPPFILES := $(wildcard ./util/*.cc ./util/*.cpp)
LIBOBJECTS := $(addsuffix .o, $(basename $(LIBCFILES)) $(basename $(LIBCPPFILES)))

# 安静模式的核心代码
ifeq ("$(origin V)", "command line")
   BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
   BUILD_VERBOSE = 0
endif

ifeq ($(BUILD_VERBOSE),0)
	QUIET_CC        = @echo '   ' CC $@;
	QUIET_CXX       = @echo '   ' CXX $@;
	QUIET_LINK      = @echo '   ' LINK $@;
	QUIET_AR        = @echo '   ' AR $@;
endif

all: $(LIBRARY) $(SHARED) 
	@echo "--------------------------make successful-----------------------"

# 进入gtest/目录下，并执行该目录下的make
check: all 
	make -C gtest

$(LIBRARY): $(LIBOBJECTS)
	-rm -rf $@
	$(QUIET_AR)$(AR) crv $@ $(LIBOBJECTS)

$(SHARED):
	$(QUIET_CXX)$(CXX) $(SHARED_LDFLAGS) -o $@ $(LIBOBJECTS) $(LIBS)

#下面的Makefile其实只是为了使用安静模式而已,如果将下面的代码去掉的话也能编译成功,因为默认的make规则将被执行
./util/%.o:./util/%.c
	$(QUIET_CC)$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

./util/%.o:./util/%.cc
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

./util/%.o:./util/%.cpp 
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

.PHONY:clean install

clean:
	-rm -f ./util/*.o  $(LIBRARY) $(SHARED) 
	make clean -C gtest
	@echo "--------------------------make clean-----------------------"

install:
	cp -r ./include/pieke /usr/local/include
	cp $(LIBRARY) $(SHARED) /usr/local/lib 
