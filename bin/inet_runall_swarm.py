#!/usr/bin/env python3
#
# Generates a makefile that executes several simulation runs
#

import argparse
import os
import sys
import subprocess

import io
import zipfile

import time
import random
import pprint

import redis
import rq
import pymongo
import gridfs


localInetRoot = os.path.abspath(os.path.dirname(os.path.realpath(__file__)) + "/..")

remoteInetRoot = "/opt/inet"

sys.path.insert(0, localInetRoot + "/tests/fingerprint")

import  fingerprints_worker



nedPath = remoteInetRoot + "/src;" + remoteInetRoot + "/examples;" + remoteInetRoot + "/showcases;" + remoteInetRoot + "/tutorials;" + remoteInetRoot + "/tests/networks;."
inetLib = remoteInetRoot + "/src/INET"
cpuTimeLimit = "30000s"
githash = ""


def localToRemoteInetPath(localPath):
    return remoteInetRoot + "/" + os.path.relpath(os.path.abspath(localPath), localInetRoot)




description = """\
Execute a number of OMNeT++ simulation runs, making use of multiple CPUs and
multiple processes. In the simplest case, you would invoke it like this:

% opp_runall ./simprog -c Foo

where "simprog" is an OMNeT++ simulation program, and "Foo" is an omnetpp.ini
configuration, containing iteration variables and/or specifying multiple
repetitions.

The first positional (non-option) argument and all following arguments are
treated as the simulation command (simulation program and its arguments).
Options intended for opp_runall should come before the the simulation command.

To limit the set of simulation runs to be performed, add a -r <runfilter>
argument to the simulation command.

opp_run runs simulations in several batches, making sure to keep all CPUs
busy. Runs of a batch execute sequentially, inside the same Cmdenv process.
The batch size as well as the number of CPUs to use can be overridden.

Command-line options:
"""

epilog = """\
Operation: opp_runall invokes "./simprog -c Foo" with the "-q runnumbers"
extra command-line arguments to figure out how many (and which) simulation
runs it needs to perform, then runs them using multiple Cmdenv processes.
opp_runall exploits GNU Make's -j option to support multiple CPUs or cores:
the commands to perform the simulation runs are written into a temporary
Makefile, and run using "make" with the appropriate -j option. The Makefile
may be exported for inspection.
"""


def unzip_bytes(zip_bytes):
    with io.BytesIO(zip_bytes) as bytestream:
        with zipfile.ZipFile(bytestream, "r") as zipf:
            zipf.extractall(".")



class Runall:


    def __init__(self):
        print("connecting to the job queue")
        conn = redis.Redis(host="172.17.0.1")
        self.build_q = rq.Queue("build", connection=conn, default_timeout=10*60*60)
        self.run_q = rq.Queue("run", connection=conn, default_timeout=10*60*60)


    def run(self):
        opts = self.parseArgs()

        origwd = os.getcwd()
        if opts.directory != None:
            self.changeDir(opts.directory)

        runNumbers = self.resolveRunNumbers(opts.simProgArgs)


        global githash

        buildStarted = time.perf_counter()
        print("queueing build job")

        buildJob = self.build_q.enqueue(fingerprints_worker.buildInet, githash)

        print("waiting for build job to end")
        while not buildJob.is_finished:
            if buildJob.is_failed:
                buildJob.refresh()
                print("ERROR: build job failed " + str(buildJob.exc_info))
                exit(1)
            try:
                time.sleep(0.1)
            except KeyboardInterrupt:
                print("interrupted, not running tests (the build will still finish)")
                exit(1)

        buildEnded = time.perf_counter()

        print("build finished, took " + str(buildEnded - buildStarted) + "s, result:" + str(buildJob.result))



        rng = random.SystemRandom()

        rng.shuffle(runNumbers)


        runJobs = []
        for rn in runNumbers:
            wd = os.getcwd()

            #if not wd.startswith('/'):
            wd = "/" + os.path.relpath(os.path.abspath(os.getcwd()), localInetRoot)


            # run the simulation
            workingdir = (remoteInetRoot + "/" + wd) if wd.startswith('/') else wd

            command = "opp_run_release " + " -n '" + nedPath + "' -l " + inetLib + " -u Cmdenv " + \
                " ".join(opts.simProgArgs) + " -r " + str(rn)

            runJob = self.run_q.enqueue(fingerprints_worker.runSimulation, githash, command, workingdir, depends_on = buildJob)

            runJobs.append(runJob)


        print("queued " + str(len(runJobs)) + " jobs")

        stop = False
        while runJobs and not stop:
            try:
                for j in runJobs[:]:

                    if j.status is None:
                        print("Test " + "tst.title" + " was removed from the queue")
                        runJobs.remove(j)
                    elif j.status == rq.job.JobStatus.FAILED:
                        j.refresh()
                        print("Test " + "tst.title" + " failed: " + str(j.exc_info))
                        runJobs.remove(j)
                    else:
                        if j.result is not None:
                            pprint.pprint(vars(j.result))

                            with pymongo.MongoClient("172.17.0.1", socketTimeoutMS=10*60*1000, connectTimeoutMS=10*60*1000, serverSelectionTimeoutMS=10*60*1000) as client:
                                gfs = gridfs.GridFS(client.opp)
                                unzip_bytes(gfs.get(j.id).read())

                            runJobs.remove(j)

                time.sleep(0.1)
            except KeyboardInterrupt:
                stop = True

        if runJobs:
            print("Cancelling " + str(len([j for j in runJobs if j.status == 'queued' or j.status == 'deferred'])) + " pending jobs...")
            for j in runJobs[:]:
                j.cancel()


        print("done")


        sys.exit(0)

    def parseArgs(self):
        currentGitRef = subprocess.check_output(["git", "-C", localInetRoot, "rev-parse", "HEAD"]).decode("utf-8")

        parser = argparse.ArgumentParser(description=description, epilog=epilog, formatter_class=argparse.RawDescriptionHelpFormatter)
        parser.add_argument('--commit', default=currentGitRef, metavar='commit')
        parser.add_argument('-C', '--directory', metavar='DIR', help='Change to the given directory before doing anything.')

        try:
            opts, argv = parser.parse_known_args()
            print(opts)
            print(argv)
            opts.simProgArgs = argv

        except IOError as e:
            print(e)
            exit(1)

        global githash
        githash = subprocess.check_output(["git", "-C", localInetRoot, "rev-parse", opts.commit.strip()]).decode("utf-8").strip()

        print("running with commit " + githash)

        return opts

    def changeDir(self, directory):
            print("changing into directory: " + directory)
            try:
                os.chdir(directory)
            except IOError as e:
                print("Cannot change directory: " + str(e))
                exit(1)

    def resolveRunNumbers(self, simProgArgs):
        tmpArgs = ["opp_run"] + simProgArgs + ["-s", "-q", "runnumbers"]
        print("running: " + " ".join(tmpArgs))
        try:
            output = subprocess.check_output(tmpArgs)
        except subprocess.CalledProcessError as e:
            print(simProgArgs[0] + " [...] -q runnumbers returned nonzero exit status")
            exit(1)
        except IOError as e:
            print("Cannot execute " + simProgArgs[0] + " [...] -q runnumbers: " + str(e))
            exit(1)

        try:
            output = output.decode("utf-8")
            runNumbers = [int(num) for num in output.split()]
            print("run numbers: " + str(runNumbers))
            return runNumbers
        except:
            print("Error parsing output of " + simProgArgs[0] + " [...] -q runnumbers")
            exit(1)



tool = Runall()
tool.run()
