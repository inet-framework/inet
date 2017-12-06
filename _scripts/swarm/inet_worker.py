#!/bin/env python3

"""
This is it
"""

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


INET_ROOT = "/opt/inet"

INET_LIB_CACHE_DIR = "/host-cache/inetlib"
INET_LIB_FILE = INET_ROOT + "/src/libINET.so"

MONGO_HOST = "mng"  # discovered using swarm internal dns


LOGGER = logging.getLogger("rq.worker")


def run_program(command, workingdir):
    LOGGER.info("running command: " + str(command))

    process = subprocess.Popen(
        command, shell=True, cwd=workingdir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    out = process.communicate()[0]
    out = re.sub("\r", "", out.decode('utf-8'))

    LOGGER.info("done, exitcode: " + str(process.returncode))

    return (process.returncode, out)


def checkout_git(git_hash):
    (exitcode, output) = run_program("git fetch", INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

    (exitcode, output) = run_program("git reset --hard " + git_hash, INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)

    (exitcode, output) = run_program("git submodule update", INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to fetch git repo:\n" + output)


# this is the first kind of job
def build_inet(git_hash):
    checkout_git(git_hash)

    client = pymongo.MongoClient(MONGO_HOST)
    gfs = gridfs.GridFS(client.opp)

    if gfs.exists(git_hash):
        return "INET build " + git_hash + " already exists, not rebuilding."

    (exitcode, output) = run_program("make makefiles", INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to make makefiles:\n" + output)

    (exitcode, output) = run_program(
        "make -j $(distcc -j) MODE=release", INET_ROOT)
    if exitcode != 0:
        raise Exception("Failed to build INET:\n" + output)

    with open(INET_LIB_FILE, "rb") as inet_lib_file:
        gfs.put(inet_lib_file, _id=git_hash)

    return "INET " + git_hash + " built and stored."


def replace_inet_lib(git_hash):
    directory = INET_LIB_CACHE_DIR + "/" + git_hash

    os.makedirs(directory, exist_ok=True)

    LOGGER.info("replacing inet lib with the one built from commit " + git_hash)

    with open(directory + "/download.lock", "wb") as lf:
        LOGGER.info(
            "starting download, or waiting for the in-progress download to finish")
        with flock.Flock(lf, flock.LOCK_EX):
            LOGGER.info("download lock acquired")
            try:
                with open(directory + "/libINET.so", "xb") as f:

                    LOGGER.info(
                        "we have just created the file, so we need to download it")
                    client = pymongo.MongoClient(MONGO_HOST)
                    gfs = gridfs.GridFS(client.opp)
                    LOGGER.info("connected, downloading")
                    f.write(gfs.get(git_hash).read())
                    LOGGER.info("download done")

            except FileExistsError:
                LOGGER.info("the file was already downloaded")

    shutil.copy(directory + "/libINET.so", INET_LIB_FILE)
    LOGGER.info("file copied to the right place")


def switch_inet_to_commit(git_hash):

    LOGGER.info("switching inet to commit " + git_hash)

    current_hash = subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=INET_ROOT).strip()

    LOGGER.info("we are currently on commit: " + current_hash)

    # only switching if not already on the same hash
    if git_hash != current_hash:
        LOGGER.info("actually doing it")

        checkout_git(git_hash)
        replace_inet_lib(git_hash)

        LOGGER.info("done")
    else:
        LOGGER.info("not switching, already on it")


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


def submit_results(result_dir):
    client = pymongo.MongoClient(MONGO_HOST)
    gfs = gridfs.GridFS(client.opp)

    results_zip = zip_directory(result_dir)

    job = rq.get_current_job()

    gfs.put(results_zip, _id=job.id)


# this is the second kind of job
def run_simulation(git_hash, command, workingdir):

    result_dir = "/results"

    os.makedirs(result_dir, exist_ok=True)

    switch_inet_to_commit(git_hash)

    LOGGER.info("running simulation")

    command += " --result-dir " + result_dir

    start_time = time.time()
    (exitcode, out) = run_program(command, workingdir)
    elapsed_time = time.time() - start_time

    LOGGER.info("sim terminated, elapsed time: " + str(elapsed_time))

    result = {"command": command, "workingdir": workingdir,
              "exitcode": exitcode, "elapsedTime": elapsed_time, "output": out}

    # process error messages
    error_lines = re.findall("<!>.*", out, re.M)
    error_msg = ""
    for err in error_lines:
        err = err.strip()
        if re.search("Fingerprint", err):
            if re.search("successfully", err):
                result['isFingerprintOK'] = True
            else:
                match = re.search(
                    "(computed|calculated): ([-a-zA-Z0-9]+(/[a-z0]+)?)", err)
                if match:
                    result['isFingerprintOK'] = False
                    result['computedFingerprint'] = match.group(2)
                else:
                    raise Exception(
                        "Cannot parse fingerprint-related error message: " + err)
        else:
            error_msg += "\n" + err
            result['cpuTimeLimitReached'] = bool(
                re.search("CPU time limit reached", err))
            match = re.search(
                r"at t=([0-9]*(\.[0-9]+)?)s, event #([0-9]+)", err)
            if match:
                result['simulatedTime'] = float(match.group(1))
                result['numEvents'] = int(match.group(3))

    result['errormsg'] = error_msg.strip()

    submit_results(result_dir)

    shutil.rmtree(result_dir)

    return result
