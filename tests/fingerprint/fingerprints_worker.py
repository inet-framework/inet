import subprocess
import os
import shutil
import re
import time
import logging
import io
import zipfile

import rq
import pymongo
import gridfs
import flock

from fingerprints import SimulationResult


INET_ROOT = "/opt/inet"
inetLibCache = "/host-cache/inetlib"
inetLibFile = INET_ROOT + "/src/libINET.so"

MONGO_HOST = "mng" # discovered using swarm internal dns

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
    (exitcode, output) = runProgram("git fetch", INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

    (exitcode, output) = runProgram("git reset --hard " + gitHash, INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

    (exitcode, output) = runProgram("git submodule update", INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

# this is the first kind of job
def buildInet(gitHash):
    checkoutGit(gitHash)

    client = pymongo.MongoClient(MONGO_HOST)
    gfs = gridfs.GridFS(client.opp)

    if gfs.exists(gitHash):
        return "INET build " + gitHash + " already exists, not rebuilding."

    (exitCode, output) = runProgram("make makefiles", INET_ROOT)
    if exitCode != 0:
        raise Exception("Failed to make makefiles:\n" + output)

    (exitCode, output) = runProgram("make -j $(distcc -j) MODE=release", INET_ROOT)
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
                    client = pymongo.MongoClient(MONGO_HOST)
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


def zip_directory(directory, exclude_dirs=[]):
    with io.BytesIO() as bytestream:
        with zipfile.ZipFile(bytestream, "w") as zipf:

            for root, dirs, files in os.walk(directory):
                dirs[:] = [d for d in dirs if d not in exclude_dirs]

                for f in files:
                    zipf.write(os.path.join(root, f))

        bytestream.seek(0)
        zipped = bytes(bytestream.read())

    return zipped

def submitResults(result_dir):
    client = pymongo.MongoClient(MONGO_HOST)
    gfs = gridfs.GridFS(client.opp)

    results_zip = zip_directory(result_dir)

    job = rq.get_current_job()

    gfs.put(results_zip, _id = job.id)


def runSimulation(gitHash, command, workingdir):

    result_dir = "/results"

    os.makedirs(result_dir, exist_ok=True)

    switchInetToCommit(gitHash)

    logger.info("running simulation")
    # run the program and log the output

    command += " --result-dir " + result_dir

    t0 = time.time()
    (exitcode, out) = runProgram(command, workingdir)
    elapsedTime = time.time() - t0

    logger.info("sim terminated, elapsedTime: " + str(elapsedTime))

    result = SimulationResult(command, workingdir, exitcode, elapsedTime=elapsedTime)

    # process error messages
    error_lines = re.findall("<!>.*", out, re.M)
    error_msg = ""
    for err in error_lines:
        err = err.strip()
        if re.search("Fingerprint", err):
            if re.search("successfully", err):
                result.isFingerprintOK = True
            else:
                match = re.search("(computed|calculated): ([-a-zA-Z0-9]+(/[a-z0]+)?)", err)
                if match:
                    result.isFingerprintOK = False
                    result.computedFingerprint = match.group(2)
                else:
                    raise Exception("Cannot parse fingerprint-related error message: " + err)
        else:
            error_msg += "\n" + err
            result.cpuTimeLimitReached = bool(re.search("CPU time limit reached", err))
            match = re.search("at t=([0-9]*(\\.[0-9]+)?)s, event #([0-9]+)", err)
            if match:
                result.simulatedTime = float(match.group(1))
                result.numEvents = int(match.group(2))

    result.errormsg = error_msg.strip()

    submitResults(result_dir)

    shutil.rmtree(result_dir)

    return result
