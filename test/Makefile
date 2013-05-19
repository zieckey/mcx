# weizili@360.cn 

CC=gcc
CXX=g++
AR=ar
ARFLAGS=cru
CURRENT_DIR=$(shell echo `pwd`)
PARENT_DIRNAME=$(shell basename $(shell dirname $(CURRENT_DIR)))
SOURCE_DIR=$(CURRENT_DIR)
CFLAGS=$(TOP_CFLAGS) -c -fPIC \
	   -Wshadow -Wcast-qual -Wcast-align \
	   -Wwrite-strings -Wsign-compare \
	   -Winvalid-pch -fms-extensions -Wall \
	   -I../3rdparty -I../include \
	   -I ../3rdparty/muduo \
	   -MMD
CPPFLAGS=$(CFLAGS) -Woverloaded-virtual -Wsign-promo -fno-gnu-keywords 
LDFLAGS=-L/usr/local/lib ../src/libmcx.a ../3rdparty/build/debug/lib/libmuduo_net.a ../3rdparty/build/debug/lib/libmuduo_base.a -lpthread 

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc, .obj/%.o, $(SRCS))
DEPS := $(patsubst .obj/%.o, .obj/%.d, $(OBJS))

TARGET=mcx

all : prepare $(TARGET) 
	$(MAKE) install

check : $(TARGET)
	./$^

$(TARGET) : $(OBJS) ../src/libmcx.a
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

-include $(DEPS)

.obj/%.o : %.cc
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -o $@

prepare : 
	$(MAKE) -C ../src
	@mkdir -p .obj

install: $(TARGET) 
#	cp -f $(TARGET) $(PREFIX)/bin

clean:
	rm -rf *.o *.d $(OBJS) $(DEPS) $(TARGET) 

