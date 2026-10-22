#!/bin/bash
# Step I2: the old tool with the CI arguments, then the new tool with the store, on one build.
cd $INET_ROOT/tests/fingerprint
rm -f *.csv.UPDATED *.csv.FAILED *.csv.ERROR
( time ./fingerprinttest -f 'tplx' -f '~tNl' -f '~tND' ) > /home/runner/work/inet/inet/i2-old-$MODE.log 2>&1
cd $INET_ROOT
( time python3 -c 'import sys; from opp_repl.main import run_fingerprint_tests_main; sys.argv[0] = "opp_run_fingerprint_tests"; run_fingerprint_tests_main()' \
    --load @opp -p inet -m $MODE --no-build --result-file /home/runner/work/inet/inet/i2-new-$MODE.json ) > /home/runner/work/inet/inet/i2-new-$MODE.log 2>&1
echo I2-DONE
