

all : prepare


prepare:
	@test -d 3rdparty/rapidjson || svn checkout http://rapidjson.googlecode.com/svn/trunk/ 3rdparty/rapidjson
	@test -d 3rdparty/muduo	    || (git clone https://github.com/chenshuo/muduo.git 3rdparty/muduo ; cd 3rdparty/muduo; sh build.sh)


.PHONY: prepare 
