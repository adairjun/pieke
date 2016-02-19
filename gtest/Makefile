# # Use 'make V=1' to see the full commands
#使用安静模式可以不看gcc,g++的参数
CC := gcc 
CXX := g++
AR := ar

INCLUDE := -I/usr/local/include -I../include -I../../rapidmsg/include
LIBS := -L/usr/local/lib -L/usr/lib64/mysql -levent -lpthread -lmysqlclient -lhiredis -lboost_system -lboost_filesystem -lprotobuf

CFLAGS := 
CPPFLAGS := -std=c++11 -O2 -g -fPIC -DNDEBUG

CFILES := $(wildcard ./*.c)
CPPFILES := $(wildcard ./*.cc ./*.cpp)
OBJECTS := $(addsuffix .o, $(basename $(CFILES)) $(basename $(CPPFILES)))
#TARGET := main master_worker
TARGET := master_worker

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

all: $(TARGET)

#main: main.o
#	$(QUIET_LINK)$(CXX) -DNDEBUG -o $@ $^ -L.. -lpieke -L../../rapidmsg -lrapidmsg

master_worker: master_worker.o master_thread.o worker_thread.o
	$(QUIET_LINK)$(CXX) -DNDEBUG -o $@ $^ -L.. -lpieke -L../../rapidmsg -lrapidmsg

#下面的Makefile其实只是为了使用安静模式而已,如果将下面的代码去掉的话也能编译成功,因为默认的make规则将被执行
%.o:%.c
	$(QUIET_CC)$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

%.o:%.cc
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

%.o:%.cpp 
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

.PHONY:clean

clean:
	-rm -f *.o $(TARGET) 
