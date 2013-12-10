#!/bin/bash
 
ilCout=0
ilErrs=0
if [ -d blackboard ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '----------------Blackboard--------------------'
    ( ( cd blackboard >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d connectionManager ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '-------------ConnectionManager----------------'
    ( ( cd connectionManager >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d baseMobility ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '----------------BaseMobility------------------'
    ( ( cd baseMobility >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d basePhyLayer ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '----------------BasePhyLayer------------------'
    ( ( cd basePhyLayer >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d decider ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '----------------DeciderTest-------------------'
    ( ( cd decider >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d coord ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '-------------------Coord----------------------'
    ( ( cd coord >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d channelInfo ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '----------------ChannelInfo-------------------'
    ( ( cd channelInfo >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d radioState ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '-----------------RadioState-------------------'
    ( ( cd radioState >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d nicTest ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '---------------NICTests(80211)----------------'
    ( ( cd nicTest >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d mapping ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '---------Mapping (may take a while)-----------'
    ( ( cd mapping >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi
if [ -d power ]; then
    ilCout=$(( $ilCout + 1 ))
    echo '-----------------Power------------------------'
    ( ( cd power >/dev/null 2>&1 && \
    ./runTests.sh $1 ) && echo "PASSED" ) || ( echo "FAILED" && false )
    st=$?
    [ x$st = x0 ] || ilErrs=$(( $ilErrs + 1 ))
fi

(( $ilErrs > 0 )) && echo "$ilErrs test(s) of $ilCout failed" && exit $ilErrs
echo "All $ilCout tests passed"


