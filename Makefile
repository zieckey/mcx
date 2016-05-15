

all : prepare
	$(MAKE) -C src

test : prepare
	$(MAKE) -C src
	$(MAKE) -C test

clean : 
	$(MAKE) clean -C src
	$(MAKE) clean -C test

prepare:
	@test -d 3rdparty/rapidjson || (git clone https://github.com/miloyip/rapidjson 3rdparty/rapidjson)
	@test -d 3rdparty/muduo	    || (git clone https://github.com/chenshuo/muduo.git 3rdparty/muduo ; cd 3rdparty/muduo; sh build.sh)


.PHONY: prepare all test clean
