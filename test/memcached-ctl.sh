#!/bin/sh

CMD=$1
PORT=$2
PWD=`sh ./dirname.sh`
MEMCACHED_PID=$PWD/memcached.pid

case $CMD in
start)
    memcached -p $PORT -P $MEMCACHED_PID -d
	;;
stop)
	kill `cat $MEMCACHED_PID`
	;;
esac

exit $ERROR

