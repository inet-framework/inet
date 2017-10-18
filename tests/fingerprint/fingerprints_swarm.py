#!/usr/bin/env python3
#
# Fingerprint-based regression tested for the INET Framework.
#
# Accepts one or more CSV files with 4 columns: working directory,
# options to opp_run, simulation time limit, expected fingerprint.
# The program runs the INET simulations in the CSV files, and
# reports fingerprint mismatches as FAILed test cases. To facilitate
# test suite maintenance, the program also creates a new file (or files)
# with the updated fingerprints.
#
# Implementation is based on Python's unit testing library, so it can be
# integrated into larger test suites with minimal effort
#
# Author: Andras Varga
#

import argparse
import glob
import os
import re
import sys
import time
import random
import unittest
import subprocess

from fingerprints import FingerprintTestCaseGenerator, SimulationTextTestResult

import redis
import rq
import fingerprints_worker


localInetRoot = os.path.abspath("../..")
remoteInetRoot = "/opt/inet"

sep = ";" if sys.platform == 'win32' else ':'
nedPath = remoteInetRoot + "/src" + sep + remoteInetRoot + "/examples" + sep + remoteInetRoot + "/showcases" + sep + remoteInetRoot + "/tutorials" + sep + remoteInetRoot + "/tests/networks"
inetLib = remoteInetRoot + "/src/INET"
cpuTimeLimit = "30000s"
logFile = "test.out"
extraOppRunArgs = ""
exitCode = 0

githash = ""

def localToRemoteInetPath(localPath):
    return remoteInetRoot + "/" + os.path.relpath(os.path.abspath(localPath), localInetRoot)

class RemoteTestSuite(unittest.BaseTestSuite):

    def __init__(self):
        super(RemoteTestSuite, self).__init__()

        # TODO: pass this, and executable as well, if custom
        # testSuite.repeat = args.repeat

        print("connecting to the job queue")
        conn = redis.Redis(host="172.17.0.1")
        self.build_q = rq.Queue("build", connection=conn, default_timeout=30*60)
        self.run_q = rq.Queue("run", connection=conn, default_timeout=30*60)



    def run(self, result):

        global extraOppRunArgs
        global githash

        result.buffered = True

        print("queueing build job")
        buildJob = self.build_q.enqueue(fingerprints_worker.buildInet, githash)

        print("waiting for build job to end")
        while not buildJob.is_finished:
            if buildJob.is_failed:
                buildJob.refresh()
                print("ERROR: build job failed " + str(buildJob.exc_info))
                exit(1)
            time.sleep(0.1)

        print("build finished: " + str(buildJob.result))

        rng = random.SystemRandom()
        cases = list(self)
        rng.shuffle(cases)

        jobToTest = dict()
        runJobs = []
        for test in cases:
            wd = test.wd

            if not wd.startswith('/'):
                wd = "/" + os.path.relpath(os.path.abspath(test.wd), localInetRoot)


            # run the simulation
            workingdir = (remoteInetRoot + "/" + wd) if wd.startswith('/') else wd

            wdname = '' + wd + ' ' + test.args
            wdname = re.sub('/', '_', wdname)
            wdname = re.sub('[\\W]+', '_', wdname)
            resultdir = workingdir + "/results/" + test.csvFile + "/" + wdname

            command = "opp_run_release " + " -n " + nedPath + " -l " + inetLib + " -u Cmdenv " + test.args + \
                ((" --sim-time-limit=" + test.simtimelimit) if (test.simtimelimit != "") else "") + \
                " \"--fingerprint=" + test.fingerprint + "\" --cpu-time-limit=" + cpuTimeLimit + \
                " --vector-recording=false --scalar-recording=true" + \
                " --result-dir=" + resultdir + \
                extraOppRunArgs

            runJob = self.run_q.enqueue(fingerprints_worker.runSimulation, githash, test.title,
                                        command, workingdir, resultdir, depends_on=buildJob)

            runJob.meta['title'] = test.title
            runJob.save_meta()

            runJobs.append(runJob)
            jobToTest[runJob] = test


        print("queued " + str(len(runJobs)) + " jobs")

        stop = False
        while runJobs and not stop:
            try:
                for j in runJobs[:]:
                    if j.status is None:
                        print("Test " + j.meta["title"] + " was removed from the queue")
                        runJobs.remove(j)
                    elif j.status == rq.job.JobStatus.FAILED:
                        j.refresh()
                        print("Test " + j.meta["title"] + " failed: " + str(j.exc_info))
                        runJobs.remove(j)
                    else:
                        if j.result is not None:
                            tst = jobToTest[j]
                            result.startTest(tst)

                            try:
                                # process the result
                                # note: fingerprint mismatch is technically NOT an error in 4.2 or before! (exitcode==0)
                                #self.storeExitcodeCallback(result.exitcode)
                                if j.result.exitcode != 0:
                                    raise Exception("runtime error:" + j.result.errormsg)
                                elif j.result.cpuTimeLimitReached:
                                    raise Exception("cpu time limit exceeded")
                                elif j.result.simulatedTime == 0 and tst.simtimelimit != '0s':
                                    raise Exception("zero time simulated")
                                elif j.result.isFingerprintOK is None:
                                    raise Exception("other")
                                elif not j.result.isFingerprintOK:
                                    try:
                                        raise Exception("some fingerprint mismatch; actual  '" + j.result.computedFingerprint + "'")
                                    except:
                                        result.addFailure(tst, sys.exc_info())
                                else:
                                    result.addSuccess(tst)
                                    # fingerprint OK:
                            except:
                                result.addError(tst, sys.exc_info())

                            result.stopTest(jobToTest[j])
                            runJobs.remove(j)

                time.sleep(0.1)
            except KeyboardInterrupt:
                stop = True

        if runJobs:
            print("Cleaning up " + str(len([j for j in runJobs if j.status == 'queued' or j.status == 'deferred'])) + " pending jobs...")
            for j in runJobs[:]:
                j.cancel()

        return result




if __name__ == "__main__":
    currentGitRef = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode("utf-8")

    parser = argparse.ArgumentParser(description='Run the fingerprint tests specified in the input files.')
    parser.add_argument('testspecfiles', nargs='*', metavar='testspecfile', help='CSV files that contain the tests to run (default: *.csv). Expected CSV file columns: workingdir, args, simtimelimit, fingerprint')
    parser.add_argument('-c', '--commit', default=currentGitRef, metavar='commit')
    parser.add_argument('-m', '--match', action='append', metavar='regex', help='Line filter: a line (more precisely, workingdir+SPACE+args) must match any of the regular expressions in order for that test case to be run')
    parser.add_argument('-x', '--exclude', action='append', metavar='regex', help='Negative line filter: a line (more precisely, workingdir+SPACE+args) must NOT match any of the regular expressions in order for that test case to be run')
    parser.add_argument('-r', '--repeat', type=int, default=1, help='number of repeating each test (default: 1)')
    parser.add_argument('-a', '--oppargs', action='append', metavar='oppargs', nargs='*', help='extra opp_run arguments without leading "--", you can terminate list with " -- " ')
    parser.add_argument('-e', '--executable', help='Determines which binary to execute (e.g. opp_run_dbg, opp_run_release)')
    args = parser.parse_args()

    if os.path.isfile(logFile):
        FILE = open(logFile, "w")
        FILE.close()

    githash = args.commit

    if not args.testspecfiles:
        args.testspecfiles = glob.glob('*.csv')

    if args.oppargs:
        for oppArgList in args.oppargs:
            for oppArg in oppArgList:
                extraOppRunArgs += " --" + oppArg

    if args.executable:
        executable = args.executable

    generator = FingerprintTestCaseGenerator()
    testcases = generator.generateFromCSV(args.testspecfiles, args.match, args.exclude, args.repeat)


    testSuite = RemoteTestSuite()
    testSuite.addTests(testcases)
    testSuite.repeat = args.repeat

    testRunner = unittest.TextTestRunner(stream=sys.stdout, verbosity=9, resultclass=SimulationTextTestResult)

    testRunner.run(testSuite)


    generator.writeUpdatedCSVFiles()
    generator.writeErrorCSVFiles()
    generator.writeFailedCSVFiles()

    print("Log has been saved to %s" % logFile)

    exit(exitCode)
