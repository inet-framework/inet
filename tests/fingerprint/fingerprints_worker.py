import subprocess
import os
import shutil
import sys
import re
import time
import logging

import pymongo
import gridfs
import flock

from fingerprints import SimulationResult

MODEL_DIR = "/tmp/model/"


inetRoot = "/opt/inet"
inetLibCache = "/host-cache/inetlib"
inetLibFile = inetRoot + "/src/libINET.so"

logFile = "test.out"

mongoHost = "mng" # discovered using swarm internal dns

INET_HASH_FILE = "/opt/current_inet_hash"


logger = logging.getLogger("rq.worker")


def runProgram(command, workingdir):
    logger.info("running command: " + str(command))

    process = subprocess.Popen(command, shell=True, cwd=workingdir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    out = process.communicate()[0]
    out = re.sub("\r", "", out.decode('utf-8'))

    logger.info("done, exitcode: " + str(process.returncode))

    return (process.returncode, out)


def checkoutGit(gitHash):
    (exitCode, output) = runProgram("git fetch", inetRoot)
    if exitCode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

    (exitCode, output) = runProgram("git reset --hard " + gitHash, inetRoot)
    if exitCode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

# this is the first kind of job
def buildInet(gitHash):
    checkoutGit(gitHash)

    client = pymongo.MongoClient(mongoHost)
    gfs = gridfs.GridFS(client.opp)

    if gfs.exists(gitHash):
        return "INET build " + gitHash + " already exists, not rebuilding."

    (exitCode, output) = runProgram("make makefiles", inetRoot)
    if exitCode != 0:
        raise Exception("Failed to make makefiles:\n" + output)

    (exitCode, output) = runProgram("make -j $(distcc -j) MODE=release", inetRoot)
    if exitCode != 0:
        raise Exception("Failed to build INET:\n" + output)

    f = open(inetLibFile, "rb")
    gfs.put(f, _id = gitHash)

    return "INET " + gitHash + " built and stored."


def replaceInetLib(gitHash):
    directory = inetLibCache + "/" + gitHash

    os.makedirs(directory, exist_ok=True)

    logger.info("replacing inet lib with the one built from commit " + gitHash)

    with open(directory + "/download.lock", "wb") as lf:
        logger.info("starting download, or waiting for the in-progress download to finish")
        with flock.Flock(lf, flock.LOCK_EX):
            logger.info("download lock acquired")
            try:
                with open(directory + "/libINET.so", "xb") as f:

                    logger.info("we have just created the file, so we need to download it")
                    client = pymongo.MongoClient(mongoHost)
                    gfs = gridfs.GridFS(client.opp)
                    logger.info("connected, downloading")
                    f.write(gfs.get(gitHash).read())
                    logger.info("download done")

            except FileExistsError:
                logger.info("the file was already downloaded")


    shutil.copy(directory + "/libINET.so", inetLibFile)
    logger.info("file copied to the right place")

def switchInetToCommit(gitHash):

    logger.info("switching inet to commit " + gitHash)

    currentHash = ""
    try:
        with open(INET_HASH_FILE, "rt") as f:
            currentHash = str(f.read()).strip()
    except:
        pass # XXX

    logger.info("we are currently on commit: " + currentHash)

    # only switching if not already on the same hash
    if gitHash != currentHash:
        logger.info("actually doing it")
        checkoutGit(gitHash)

        replaceInetLib(gitHash)

        try:
            with open(INET_HASH_FILE, "wt") as f:
                f.write(gitHash)
        except:
            pass # XXX

        logger.info("done")
    else:
        logger.info("not switching, already on it")


def runSimulation(gitHash, title, command, workingdir, resultdir):

    os.makedirs(resultdir, exist_ok=True)

    switchInetToCommit(gitHash)

    global logFile
    os.makedirs(workingdir + "/results", exist_ok=True)

    logger.info("running simulation")
    # run the program and log the output
    t0 = time.time()

    (exitcode, out) = runProgram(command, workingdir)

    elapsedTime = time.time() - t0

    logger.info("sim terminated, elapsedTime: " + str(elapsedTime))

    FILE = open(logFile, "a")
    FILE.write("------------------------------------------------------\n"
             + "Running: " + title + "\n\n"
             + "$ cd " + workingdir + "\n"
             + "$ " + command + "\n\n"
             + out.strip() + "\n\n"
             + "Exit code: " + str(exitcode) + "\n"
             + "Elapsed time:  " + str(round(elapsedTime,2)) + "s\n\n")
    FILE.close()

    FILE = open(resultdir + "/test.out", "w")
    FILE.write("------------------------------------------------------\n"
             + "Running: " + title + "\n\n"
             + "$ cd " + workingdir + "\n"
             + "$ " + command + "\n\n"
             + out.strip() + "\n\n"
             + "Exit code: " + str(exitcode) + "\n"
             + "Elapsed time:  " + str(round(elapsedTime,2)) + "s\n\n")
    FILE.close()

    result = SimulationResult(command, workingdir, exitcode, elapsedTime=elapsedTime)

    # process error messages
    errorLines = re.findall("<!>.*", out, re.M)
    errorMsg = ""
    for err in errorLines:
        err = err.strip()
        if re.search("Fingerprint", err):
            if re.search("successfully", err):
                result.isFingerprintOK = True
            else:
                m = re.search("(computed|calculated): ([-a-zA-Z0-9]+(/[a-z0]+)?)", err)
                if m:
                    result.isFingerprintOK = False
                    result.computedFingerprint = m.group(2)
                else:
                    raise Exception("Cannot parse fingerprint-related error message: " + err)
        else:
            errorMsg += "\n" + err
            if re.search("CPU time limit reached", err):
                result.cpuTimeLimitReached = True
            m = re.search("at event #([0-9]+), t=([0-9]*(\\.[0-9]+)?)", err)
            if m:
                result.numEvents = int(m.group(1))
                result.simulatedTime = float(m.group(2))

    result.errormsg = errorMsg.strip()
    return result
