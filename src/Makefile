# weizili@360.cn 

CC=gcc
CXX=g++
AR=ar
ARFLAGS=cru
CURRENT_DIR=$(shell echo `pwd`)
SOURCE_DIR=$(CURRENT_DIR)
PREFIX=${CURRENT_DIR}/../../install
CFLAGS=$(TOP_CFLAGS) -g -c -fPIC \
	   -Wall -Wno-unused-parameter -Wno-unused-function -Wunused-variable -Wunused-value \
	   -Wshadow -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare \
	   -Winvalid-pch -fms-extensions -Wfloat-equal -Wextra \
	   -I .. -I ../3rdparty \
	   -I ../3rdparty/muduo \
	   -I/usr/local/include  -MMD
CPPFLAGS= $(CFLAGS) -Woverloaded-virtual -Wsign-promo -fno-gnu-keywords
LDFLAGS=$(TOP_LDFLAGS) -L/usr/local/lib -lpthread 
LIBNAME=mcxmemcached

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc, .obj/%.o, $(SRCS)) 
DEPS := $(patsubst .obj/%.o, .obj/%.d, $(OBJS))

TARGET_SO=lib$(LIBNAME).so
TARGET_A=lib$(LIBNAME).a

all : prepare $(TARGET_SO) $(TARGET_A)

$(TARGET_A) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) 
	ranlib $@			   

$(TARGET_SO) : $(OBJS)
	$(CXX) $(LDFLAGS)  $(OBJS) -shared -o $@
	
check : $(TARGET_A) $(TARGET_SO)	
	$(MAKE) check -C test
	$(MAKE) check -C quick_test

prepare : 
	@mkdir -p .obj

.obj/%.o : %.cc
	$(CXX) $(CPPFLAGS) $< -o $@

-include $(DEPS)

t:
	@echo "LIBNAME:" $(LIBNAME)
	@echo "SRCS:" $(SRCS)
	@echo "OBJS:" $(OBJS)
	@echo "CFLAGS:" $(CFLAGS)
	@echo "CPPFLAGS:" $(CPPFLAGS)

install: $(TARGET_A) $(TARGET_SO) 
	cp -f $(TARGET_A) $(TARGET_SO) $(PREFIX)/lib

clean:
	rm -rf $(OBJS) $(DEPS) $(TARGET_SO) $(TARGET_A)
#	$(MAKE) clean -C test

