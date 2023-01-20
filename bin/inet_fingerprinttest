#!/usr/bin/env python3
#
# Fingerprint-based regression test tool
#
# Accepts one or more CSV files with 6 columns: working directory,
# command to run, simulation time limit, expected fingerprint,
# expected result, tags.
# The program runs the simulations in the CSV files, and
# reports fingerprint mismatches as FAILed test cases. To facilitate
# test suite maintenance, the program also creates a new file (or files)
# with the updated fingerprints.
#
# Implementation is based on Python's unit testing library, so it can be
# integrated into larger test suites with minimal effort
#
# Authors: Andras Varga, Zoltan Bojthe
#

from typing import List
import argparse
import copy
import csv
import glob
import multiprocessing
import os
import re
import subprocess
import sys
import threading
import time
import traceback
import unittest
from io import StringIO

oppErrorCodeMax = 10
rootDir = os.path.abspath(".")  # the working directory in the CSV file is relative to this dir
cpuTimeLimit = "600s"
logFile = "test.out"
extraOppRunArgs = ""
debug=False
release=False
exitCode = 0
writeOutfile = True
useColors = sys.stdout.isatty()

defaultFingerprintCalculator = 'inet::FingerprintCalculator'

txtPASS = "PASS"
txtPASS_unexpected = "PASS (unexpected)"
txtFAILED = "FAILED"
txtFAILED_expected = "FAILED (expected)"
txtERROR = "ERROR"
txtERROR_expected = "ERROR (expected)"

if useColors:
    txtPASS = '\033[0;32m' + "PASS" + "\033[0;0m"     # GREEN
    txtPASS_unexpected = '\033[0;32m' + "PASS" + "\033[0;0m" + " " + "\033[1;31m" + "(unexpected)" + "\033[0;0m"     # GREEN + RED
    txtFAILED = "\033[1;33m" + "FAILED" + "\033[0;0m"    # YELLOW
    txtFAILED_expected = "\033[2;32m" + "FAILED (expected)" + "\033[0;0m"    # DARK GREEN
    txtERROR = "\033[1;31m" + "ERROR" + "\033[0;0m"      # RED
    txtERROR_expected = "\033[2;32m" + "ERROR (expected)" + "\033[0;0m"    # DARK GREEN

fpExtraArgs = {
    "~tND": { "'--**.crcMode=\"computed\"'",
              "'--**.fcsMode=\"computed\"'"
            },
    "tyf" : { "--cmdenv-fake-gui=true",
              "--cmdenv-fake-gui-before-event-probability=0.1",
              "--cmdenv-fake-gui-after-event-probability=0.1",
              "--cmdenv-fake-gui-on-hold-probability=0.1",
              "--cmdenv-fake-gui-on-hold-numsteps=3",
              "--cmdenv-fake-gui-on-simtime-probability=0.1",
              "--cmdenv-fake-gui-on-simtime-numsteps=3",
              "'--**.fadeOutMode=\"animationTime\"'",
              "'--**.signalAnimationSpeedChangeTimeMode=\"animationTime\"'"
            }
}


def formatFingerprintGroupForCsv(fingerprintGroup : List[List[str]]):
    """
    Formatting the fingerprint group for csv file.
    Example fingerprintGroup: [ ['1234-5678/tplx', ['2345-6789/tplx']], ['0123-abcd/tl', '2345-6789/tl']]
    """
    return ';'.join(' '.join(sorted(x)) for x in fingerprintGroup)


def formatFingerprintGroupForOmnetpp(fingerprintGroup : List[List[str]]) -> str :
    """ formatting the fingerprint group for omnetpp command line argument """
    return ', '.join(' '.join(sorted(x)) for x in fingerprintGroup)


def parseFingerprintGroupFromCsv(str : str) -> List[List[str]] :
    return list(list(x.split(' ')) for x in str.split(';'))


def parseFingerprintGroupFromOmnetpp(str : str) -> List[List[str]] :
    return list(list(x.split(' ')) for x in str.split(', '))


def filterFingerprintGroup(fingerprintGroup : List[List[str]], includedFingerprintTypes : List[str], excludedFingerprintTypes : List[str]) -> List[List[str]]:
    """ returns filtered out fingerprint group"""
    if includedFingerprintTypes:
        l1 = list()
        for i in range(len(fingerprintGroup)):
            s = list(filter(lambda x : x.split('/', 1)[1] in includedFingerprintTypes, fingerprintGroup[i]))
            if s:
                l1.append(s)
    else:
        l1 = fingerprintGroup

    if excludedFingerprintTypes:
        l2 = list()
        for i in range(len(l1)):
            s = list(filter(lambda x : x.split('/', 1)[1] not in excludedFingerprintTypes, l1[i]))
            if s:
                l2.append(s)
    else:
        l2 = l1

    return l2


def mergeFingerprintListIntoGroup(fingerprintGroup : List[List[str]], fingerprintList : List[str]):
    for i in range(len(fingerprintList)):
        if i >= len(fingerprintGroup):
            fingerprintGroup.append(list())
        s = list(filter(lambda x : x not in fingerprintGroup[i], fingerprintList[i]))
        if s:
            fingerprintGroup[i].extend(s)
        #mFp[i].append(otherFp[i])  # TODO filter to unique


def diffFpGroups(actualFpGroup: List[List[str]], expectedFpGroup: List[List[str]]):
    actFp = actualFpGroup.copy()
    expFp = expectedFpGroup.copy()
    for act in actualFpGroup:
        for exp in expectedFpGroup:
            if len(set(exp) & set(act)) > 0:
                actFp.remove(act)
                expFp.remove(exp)
#    actFp = [fp for fp in actualFpGroup if all(fp_elem not in expectedFpGroup for fp_elem in fp)]
#    expFp = [fp for fp in expectedFpGroup if all(fp_elem not in actualFpGroup for fp_elem in fp)]
    return (actFp, expFp)


class FingerprintTestCaseGenerator():
    fileToSimulationsMap = {}
    def generateFromCSV(self, csvFileList, fpFilterList, excludeFpFilterList, filterRegexList, excludeFilterRegexList, repeat):
        testcases = []
        for csvFile in csvFileList:
            simulations = self.parseSimulationsTable(csvFile)
            self.fileToSimulationsMap[csvFile] = simulations
            testcases.extend(self.generateFromDictList(simulations, fpFilterList, excludeFpFilterList, filterRegexList, excludeFilterRegexList, repeat))
        return testcases

    def generateFromDictList(self, simulations, fpFilterList, excludeFpFilterList, filterRegexList, excludeFilterRegexList, repeat):
        class StoreFingerprintCallback:
            def __init__(self, simulation):
                self.simulation = simulation

            def __call__(self, fingerprint):
                self.simulation['computedFingerprint'] = fingerprint

        class StoreExitcodeCallback:
            def __init__(self, simulation):
                self.simulation = simulation

            def __call__(self, exitcode):
                self.simulation['exitcode'] = exitcode

        testcases = []
        for simulation in simulations:
            title = simulation['wd'] + " " + simulation['args'] + " " + simulation['tags']
            if not filterRegexList or ['x' for regex in filterRegexList if re.search(regex, title)]: # if any regex matches title
                if not excludeFilterRegexList or not ['x' for regex in excludeFilterRegexList if re.search(regex, title)]: # if NO exclude-regex matches title
                    fingerprints = parseFingerprintGroupFromCsv(simulation['fingerprint'])
                    if len(fpFilterList) > 0 or len(excludeFpFilterList) > 0:
                        fingerprints = filterFingerprintGroup(fingerprints, fpFilterList, excludeFpFilterList)
                    if len(fingerprints):
                        fpArg = set()
                        fpSet = set()
                        for fps in fingerprints:
                            for fp in fps:
                                fpKey = re.sub(r'[a-fA-F0-9]{4}-[a-fA-F0-9]{4}/', '', fp)
                                fpSet.add(fpKey)
                        for fps in fpSet:
                            if (fps in fpExtraArgs):
                                fpArg = fpArg.union(fpExtraArgs[fps])
                        simulation['expectedFingerprint'] = fingerprints
                        testcases.append(FingerprintTestCase(title, simulation['file'], simulation['wd'], simulation['args'], " ".join(fpArg),
                                simulation['simtimelimit'], fingerprints, simulation['expectedResult'], StoreFingerprintCallback(simulation), StoreExitcodeCallback(simulation), repeat))
        return testcases

    def commentRemover(self, csvData):
        p = re.compile(' *#.*$')
        for line in csvData:
            yield p.sub('',line.decode('utf-8'))

    # parse the CSV into a list of dicts
    def parseSimulationsTable(self, csvFile):
        simulations = []
        f = open(csvFile, 'rb')
        csvReader = csv.reader(self.commentRemover(f), delimiter=str(','), quotechar=str('"'), skipinitialspace=True)
        for fields in csvReader:
            if len(fields) == 0:
                pass        # empty line
            elif len(fields) == 6:
                if fields[4] in ['PASS', 'FAIL', 'ERROR']:
                    simulations.append({
                            'file': csvFile,
                            'line' : csvReader.line_num,
                            'wd': fields[0],
                            'args': fields[1],
                            'simtimelimit': fields[2],
                            'fingerprint': fields[3],
                            'expectedResult': fields[4],
                            'tags': fields[5]
                            })
                else:
                    raise Exception(csvFile + " Line " + str(csvReader.line_num) + ": the 5th item must contain one of 'PASS', 'FAIL', 'ERROR'" + ": " + '"' + '", "'.join(fields) + '"')
            else:
                raise Exception(csvFile + " Line " + str(csvReader.line_num) + " must contain 6 items, but contains " + str(len(fields)) + ": " + '"' + '", "'.join(fields) + '"')
        f.close()
        return simulations

    def writeUpdatedCSVFiles(self):
        for csvFile, simulations in self.fileToSimulationsMap.items():
            updatedContents = self.formatUpdatedSimulationsTable(csvFile, simulations)
            if updatedContents:
                updatedFile = csvFile + ".UPDATED"
                ff = open(updatedFile, 'w')
                ff.write(updatedContents)
                ff.close()
                print("Check " + updatedFile + " for updated fingerprints")

    def writeFailedCSVFiles(self):
        for csvFile, simulations in self.fileToSimulationsMap.items():
            failedContents = self.formatFailedSimulationsTable(csvFile, simulations)
            if failedContents:
                failedFile = csvFile + ".FAILED"
                ff = open(failedFile, 'w')
                ff.write(failedContents)
                ff.close()
                print("Check " + failedFile + " for failed fingerprints")

    def writeErrorCSVFiles(self):
        for csvFile, simulations in self.fileToSimulationsMap.items():
            errorContents = self.formatErrorSimulationsTable(csvFile, simulations)
            if errorContents:
                errorFile = csvFile + ".ERROR"
                ff = open(errorFile, 'w')
                ff.write(errorContents)
                ff.close()
                print("Check " + errorFile + " for errors")

    def escape(self, str):
        if re.search(r'[\r\n\",]', str):
            str = '"' + re.sub('"','""',str) + '"'
        return str

    def formatUpdatedSimulationsTable(self, csvFile, simulations):
        # if there is a computed fingerprint, print that instead of existing one
        ff = open(csvFile, 'r')
        lines = ff.readlines()
        ff.close()
        lines.insert(0, '')     # csv line count is 1..n; insert an empty item --> lines[1] is the first line

        containsComputedFingerprint = False
        for simulation in simulations:
            if 'computedFingerprint' in simulation:
                oldFingerprint = simulation['expectedFingerprint']
                newFingerprint = simulation['computedFingerprint']

                containsComputedFingerprint = True
                line = simulation['line']

                if len(oldFingerprint) == len(newFingerprint):
                    newLine = lines[line]
                    errFlag = False
                    for i in range(len(oldFingerprint)):
                        if len(set(oldFingerprint[i]) & set(newFingerprint[i])) == 0:
                            oldFpStr = ' '.join(oldFingerprint[i])
                            newFpStr = ' '.join(newFingerprint[i])
                            patternStr = "\\b" + oldFpStr + "\\b"
                            pattern = re.compile(patternStr)
                            #print("INFO: replace fingerprint '%s' to '%s' at '%s' line %d:\n     %s" % (oldFpStr, newFpStr, csvFile, line, newLine))
                            (newLine, cnt) = pattern.subn(newFpStr, newLine)
                            if (cnt != 1):
                                errFlag = True
                                print("ERROR: Cannot replace fingerprint '%s' to '%s' at '%s' line %d (cnt: %d):\n     %s" % (oldFpStr, newFpStr, csvFile, line, cnt, lines[line]))
                    lines[line] = newLine
                else:
                    print("ERROR: Cannot replace fingerprint '%s' to '%s' at '%s' line %d:\n     %s" % (formatFingerprintGroupForCsv(oldFingerprint), formatFingerprintGroupForCsv(newFingerprint), csvFile, line, lines[line]))

        return ''.join(lines) if containsComputedFingerprint else None

    def formatFailedSimulationsTable(self, csvFile, simulations):
        ff = open(csvFile, 'r')
        lines = ff.readlines()
        ff.close()
        lines.insert(0, '')     # csv line count is 1..n; insert an empty item --> lines[1] is the first line
        result = []

        containsFailures = False
        for simulation in simulations:
            if 'computedFingerprint' in simulation:
                oldFingerprint = simulation['expectedFingerprint']
                newFingerprint = simulation['computedFingerprint']
                if oldFingerprint != newFingerprint:
                    if not containsFailures:
                        containsFailures = True
                        result.append("# Failures:\n")
                    result.append(lines[simulation['line']])
        return ''.join(result) if containsFailures else None

    def formatErrorSimulationsTable(self, csvFile, simulations):
        ff = open(csvFile, 'r')
        lines = ff.readlines()
        ff.close()
        lines.insert(0, '')     # csv line count is 1..n; insert an empty item --> lines[1] is the first line
        result = []

        containsErrors = False
        for simulation in simulations:
            if 'exitcode' in simulation and simulation['exitcode'] != 0:
                if not containsErrors:
                    containsErrors = True
                    result.append("# Errors:\n")
                result.append(lines[simulation['line']])

        return ''.join(result) if containsErrors else None


class SimulationResult:
    def __init__(self, command, workingdir, exitcode, errorMsg=None, isFingerprintOK=None,
            computedFingerprint=None, simulatedTime=None, numEvents=None, elapsedTime=None, cpuTimeLimitReached=None):
        self.command = command
        self.workingdir = workingdir
        self.exitcode = exitcode
        self.errorMsg = errorMsg
        self.isFingerprintOK = isFingerprintOK
        self.computedFingerprint = computedFingerprint
        self.simulatedTime = simulatedTime
        self.numEvents = numEvents
        self.elapsedTime = elapsedTime
        self.cpuTimeLimitReached = cpuTimeLimitReached


class SimulationTestCase(unittest.TestCase):
    def runSimulation(self, title, command, workingdir, resultdir):
        global logFile
        ensure_dir(workingdir + "/results")

        # run the program and log the output
        t0 = time.time()

        (exitcode, out) = self.runProgram(command, workingdir, resultdir)

        elapsedTime = time.time() - t0
        FILE = open(logFile, "a")
        FILE.write("------------------------------------------------------\n"
                 + "Running: " + title + "\n\n"
                 + "$ cd " + workingdir + "\n"
                 + "$ " + command + "\n\n"
                 + out.strip() + "\n\n"
                 + "Exit code: " + str(exitcode) + "\n"
                 + "Elapsed time:  " + str(round(elapsedTime,2)) + "s\n\n")
        FILE.close()

        if (writeOutfile):
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
        fp = '[a-fA-F0-9]{4}-[a-fA-F0-9]{4}/[a-zA-Z0~]+'
        fps = fp + '(?: '+fp+')*'
        fpsl = fps + '(?:, '+fps+')*'
        pattern = 'calculated: (' + fpsl + '), expected: (' + fpsl + ')( -- during finalization)?$'
        isFingerprintPass = False
        isFingerprintFail = False
        wasError = False
        result.exitcode = 0

        for err in errorLines:
            err = err.strip()
            if re.search("Fingerprint", err):
                if re.search("Fingerprint successfully", err):
                    isFingerprintPass = True
                    result.isFingerprintOK = True
                else:
                    m = re.search(pattern, err)
                    if m:
                        isFingerprintFail = True
                        result.computedFingerprint = parseFingerprintGroupFromOmnetpp(m.group(1))
                        result.expectedFingerprint = parseFingerprintGroupFromOmnetpp(m.group(2))
                        # TODO only the last failed run's fingerprints stored
                    else:
                        raise Exception("Cannot parse fingerprint-related error message: " + err + "\nPattern: >>>"+pattern+'<<<')
            elif re.search("Simulation time limit reached", err):
                pass
            else:
                errorMsg += "\n" + err
                wasError = True
                if re.search("CPU time limit reached", err):
                    result.cpuTimeLimitReached = True
                m = re.search(r"at t=([0-9]*(\.[0-9]+)?)s, event #([0-9]+)", err)
                if m:
                    result.simulatedTime = float(m.group(1))
                    result.numEvents = int(m.group(3))

        if exitcode > oppErrorCodeMax: # smallest system error code
            wasError = True
            errorMsg += "\nExit code: " + str(exitcode)
#            try:
#                errorMsg += " " + os.strerror(exitcode-128)
#            except ValueError:
#                pass    # do nothing

        if wasError:
            result.exitcode = exitcode
        elif result.cpuTimeLimitReached:
            result.exitcode = 1
        elif isFingerprintFail and isFingerprintPass:
            result.isFingerprintOK = False
            errorMsg += "\nFound PASSED and FAILED fingerprint results, too."
        elif isFingerprintFail:
            result.isFingerprintOK = False
        elif isFingerprintPass:
            result.isFingerprintOK = True
        else:
            pass

        result.errorMsg = errorMsg.strip()
        return result

    def runProgram(self, command, workingdir, resultdir):
        env = os.environ
#        env['CPUPROFILE'] = resultdir+"/cpuprofile"
#        env['CPUPROFILE_FREQUENCY'] = "1000"
        process = subprocess.Popen(['sh','-c',command], shell=sys.platform.startswith('win'), cwd=workingdir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
        out = process.communicate()[0]
        out = re.sub("\r", "", out.decode('utf-8'))
        return (process.returncode, out)


class FingerprintTestCase(SimulationTestCase):
    def __init__(self, title, csvFile, wd, cmdLine, extraFpArg, simtimelimit, fingerprint, expectedResult, storeFingerprintCallback, storeExitcodeCallback, repeat):
        SimulationTestCase.__init__(self)
        self.title = title
        self.csvFile = csvFile
        self.wd = wd
        self.cmdLine = cmdLine
        self.extraFpArg = extraFpArg
        self.simtimelimit = simtimelimit
        self.fingerprint = fingerprint
        self.expectedResult = expectedResult
        self.storeFingerprintCallback = storeFingerprintCallback
        self.storeExitcodeCallback = storeExitcodeCallback
        self.repeat = repeat

    def runTest(self):
        # CPU time limit is a safety guard: fingerprint checks shouldn't take forever
        global rootDir, executable, debug, release, cpuTimeLimit, extraOppRunArgs


        # run the simulation
        workingdir = _iif(self.wd.startswith('/'), rootDir + "/" + self.wd, self.wd)
        wdname = '' + self.wd + ' ' + self.cmdLine
        wdname = re.sub('/', '_', wdname)
        wdname = re.sub('[\W]+', '_', wdname)
        resultdir = os.path.abspath(".") + "/results/" + self.csvFile + "/" + wdname
        ensure_dir(resultdir)

        # Check if the command line does not contain executable name (starts with an option i.e. - char)
        # and use the executable name from the command line.
        # Otherwise, assume the first word as the name of the executable.
        (exeName, progArgs) = (executable, self.cmdLine) if (self.cmdLine.startswith("-")) else self.cmdLine.split(None, 1)

        command = (exeName + "_dbg" if debug else exeName + "_release" if release else exeName) + " -u Cmdenv " + progArgs + ' ' + self.extraFpArg + \
            _iif(self.simtimelimit != "", " --sim-time-limit=" + self.simtimelimit, "") + \
            " \"--fingerprint=" + formatFingerprintGroupForOmnetpp(self.fingerprint) + "\" --cpu-time-limit=" + cpuTimeLimit + \
            " --vector-recording=false --scalar-recording=true" + \
            " --result-dir=" + resultdir + \
            " " + extraOppRunArgs

        # print("COMMAND: " + command + '\n')
        anyFingerprintBad = False
        computedFingerprints = list()
        for rep in range(self.repeat):
            curFp = list()
            result = self.runSimulation(self.title, command, workingdir, resultdir)

            # process the result
            # note: fingerprint mismatch is technically NOT an error in omnetpp 6.0 or before! (exitcode==0)
            # note: fingerprint mismatch is an error in omnetpp 6.1 or after! (exitcode==1)
            self.storeExitcodeCallback(result.exitcode)
            if result.exitcode != 0:
                raise Exception("runtime error with exitcode="+str(result.exitcode)+": " + result.errorMsg)
            elif result.cpuTimeLimitReached:
                raise Exception("cpu time limit exceeded")
            elif result.simulatedTime == 0 and self.simtimelimit != '0s':
                raise Exception("zero time simulated")
            elif result.isFingerprintOK is None:
                raise Exception("other")
            elif result.isFingerprintOK == False:
                curFp = result.computedFingerprint
                anyFingerprintBad = True
            else:
                # fingerprint OK:
                curFp = self.fingerprint

            if len(curFp):
                mergeFingerprintListIntoGroup(computedFingerprints, curFp)

        if anyFingerprintBad:
            self.storeFingerprintCallback(computedFingerprints)
            (afp, efp) = diffFpGroups(computedFingerprints, self.fingerprint)
            assert False, "some fingerprint mismatch: actual '" + formatFingerprintGroupForCsv(afp) + "', expected: '" + formatFingerprintGroupForCsv(efp) + "'"

    def __str__(self):
        return self.title

class ThreadSafeIter:
    """Takes an iterator/generator and makes it thread-safe by
    serializing call to the `next` method of given iterator/generator.
    """
    def __init__(self, it):
        self.it = it
        self.lock = threading.Lock()

    def __iter__(self):
        return self

    def __next__(self):
        with self.lock:
            return next(self.it)

    next = __next__  # for python 2 compatibility



class ThreadedTestSuite(unittest.BaseTestSuite):
    """ runs toplevel tests in n threads
    """

    # How many test process at the time.
    thread_count = multiprocessing.cpu_count()

    def run(self, result):
        it = ThreadSafeIter(self.__iter__())

        result.buffered = True

        threads = []

        for i in range(self.thread_count):
            # Create self.thread_count number of threads that together will
            # cooperate removing every ip in the list. Each thread will do the
            # job as fast as it can.
            t = threading.Thread(target=self.runThread, args=(result, it))
            t.daemon = True
            t.start()
            threads.append(t)

        # Wait until all the threads are done. .join() is blocking.
        #for t in threads:
        #    t.join()
        runApp = True
        while runApp and threading.active_count() > 1:
            try:
                time.sleep(0.1)
            except KeyboardInterrupt:
                runApp = False
        return result

    def runThread(self, result, it):
        tresult = result.startThread()
        for test in it:
            if result.shouldStop:
                break
            test(tresult)
        tresult.stopThread()


class ThreadedTestResult(unittest.TestResult):
    """TestResult with threads
    """

    def __init__(self, stream=None, descriptions=None, verbosity=None):
        super(ThreadedTestResult, self).__init__()
        self.parent = None
        self.lock = threading.Lock()

    def startThread(self):
        ret = copy.copy(self)
        ret.parent = self
        return ret

    def stop():
        super(ThreadedTestResult, self).stop()
        if self.parent:
            self.parent.stop()

    def stopThread(self):
        if self.parent == None:
            return 0
        self.parent.testsRun += self.testsRun
        return 1

    def startTest(self, test):
        "Called when the given test is about to be run"
        super(ThreadedTestResult, self).startTest(test)
        self.oldstream = self.stream
        self.stream = StringIO()

    def stopTest(self, test):
        """Called when the given test has been run"""
        super(ThreadedTestResult, self).stopTest(test)
        out = self.stream.getvalue()
        with self.lock:
            self.stream = self.oldstream
            self.stream.write(out)


#
# Copy/paste of TextTestResult, with minor modifications in the output:
# we want to print the error text after ERROR and FAIL, but we don't want
# to print stack traces.
#
class SimulationTextTestResult(ThreadedTestResult):
    """A test result class that can print formatted text results to a stream.

    Used by TextTestRunner.
    """
    separator1 = '=' * 70
    separator2 = '-' * 70

    def __init__(self, stream, descriptions, verbosity):
        super(SimulationTextTestResult, self).__init__()
        self.stream = stream
        self.showAll = verbosity > 1
        self.dots = verbosity == 1
        self.descriptions = descriptions
        self.expectedErrors = []

    def getDescription(self, test):
        doc_first_line = test.shortDescription()
        if self.descriptions and doc_first_line:
            return '\n'.join((str(test), doc_first_line))
        else:
            return str(test)

    def startTest(self, test):
        super(SimulationTextTestResult, self).startTest(test)
        if self.showAll:
            self.stream.write(self.getDescription(test))
            self.stream.write(" ... ")
            self.stream.flush()

    def addSuccess(self, test):
        super(SimulationTextTestResult, self).addSuccess(test)
        if test.expectedResult == 'PASS':
            if self.showAll:
                self.stream.write(": " + txtPASS + "\n")
            elif self.dots:
                self.stream.write('.')
                self.stream.flush()
        else:
            self.addUnexpectedSuccess(test)
    def addError(self, test, err):
        # modified
        if test.expectedResult == 'ERROR':
            self.addExpectedError(test, err)
        else:
            super(SimulationTextTestResult, self).addError(test, err)
            errmsg = err[1]

            # for complete error msg (e.g. error in python code):
            # errmsg = traceback.format_exception(err[0], err[1], err[2])

            self.errors[-1] = (test, errmsg)  # super class method inserts stack trace; we don't need that, so overwrite it
            if self.showAll:
                self.stream.write(": " + txtERROR + " (should be %s): %s\n" % (test.expectedResult, errmsg))
            elif self.dots:
                self.stream.write('E')
                self.stream.flush()
            global exitCode
            exitCode = 1   # result is not the expected result

    def addExpectedError(self, test, err):
        self.expectedErrors.append((test, self._exc_info_to_string(err, test)))
        self._mirrorOutput = True
        self.expectedErrors[-1] = (test, err[1])  # super class method inserts stack trace; we don't need that, so overwrite it
        if self.showAll:
            self.stream.write(": " + txtERROR_expected + "\n")
        elif self.dots:
            self.stream.write('e')
            self.stream.flush()

    def addFailure(self, test, err):
        # modified
        if test.expectedResult == 'FAIL':
            self.addExpectedFailure(test, err)
        else:
            super(SimulationTextTestResult, self).addFailure(test, err)
            errmsg = err[1]
            self.failures[-1] = (test, errmsg)  # super class method inserts stack trace; we don't need that, so overwrite it
            if self.showAll:
                self.stream.write(": " + txtFAILED + " (should be %s): %s\n" % (test.expectedResult, errmsg))
            elif self.dots:
                self.stream.write('F')
                self.stream.flush()
            global exitCode
            exitCode = 1   # result is not the expected result

    def addSkip(self, test, reason):
        super(SimulationTextTestResult, self).addSkip(test, reason)
        if self.showAll:
            self.stream.write(": skipped {0!r}".format(reason))
            self.stream.write("\n")
        elif self.dots:
            self.stream.write("s")
            self.stream.flush()

    def addExpectedFailure(self, test, err):
        super(SimulationTextTestResult, self).addExpectedFailure(test, err)
        self.expectedFailures[-1] = (test, err[1])  # super class method inserts stack trace; we don't need that, so overwrite it
        if self.showAll:
            self.stream.write(": " + txtFAILED_expected + "\n")
        elif self.dots:
            self.stream.write("x")
            self.stream.flush()

    def addUnexpectedSuccess(self, test):
        super(SimulationTextTestResult, self).addUnexpectedSuccess(test)
        self.unexpectedSuccesses[-1] = (test)  # super class method inserts stack trace; we don't need that, so overwrite it
        if self.showAll:
            self.stream.write(": " + txtPASS_unexpected + "\n")
        elif self.dots:
            self.stream.write("u")
            self.stream.flush()
        global exitCode
        exitCode = 1   # result is not the expected result

    def printErrors(self):
        # modified
        if self.dots or self.showAll:
            self.stream.write("\n")
        self.printErrorList('Errors', self.errors)
        self.printErrorList('Failures', self.failures)
        self.printUnexpectedSuccessList('Unexpected successes', self.unexpectedSuccesses)
        self.printErrorList('Expected errors', self.expectedErrors)
        self.printErrorList('Expected failures', self.expectedFailures)

    def printErrorList(self, flavour, errors):
        # modified
        if errors:
            self.stream.write("%s:\n" % flavour)
        for test, err in errors:
            self.stream.write("  %s (%s)\n" % (self.getDescription(test), err))

    def printUnexpectedSuccessList(self, flavour, errors):
        if errors:
            self.stream.write("%s:\n" % flavour)
        for test in errors:
            self.stream.write("  %s\n" % (self.getDescription(test)))

def _iif(cond,t,f):
    return t if cond else f

def ensure_dir(f):
    try:
        os.makedirs(f)
    except:
        pass # do nothing if already exist

if __name__ == "__main__":
    defaultNumThreads = multiprocessing.cpu_count()
    if defaultNumThreads >= 6:
        defaultNumThreads = defaultNumThreads - 1
    parser = argparse.ArgumentParser(description='Run the fingerprint tests specified in the input files.')
    parser.add_argument('testspecfiles', nargs='*', metavar='testspecfile', help='CSV files that contain the tests to run (default: *.csv). Expected CSV file columns: working directory, command to run, simulation time limit, expected fingerprint, expected result, tags. The command column may contain only options without a program name (i.e. it starts with - ). In this case the --executable option can be used to specify a program name.')
    parser.add_argument('-f', '--fpfilter', default=list(), action='append', metavar='ingredient', help='fingerprint ingredient filter (expected values after the "/" character)')
    parser.add_argument('-F', '--excludefpfilter', default=list(), action='append', metavar='ingredient', help='fingerprint ingredient excluding filter (excluded values after the "/" character)')
    parser.add_argument('-m', '--match', action='append', metavar='regex', help='Line filter: a line (more precisely, workingdir+SPACE+args) must match any of the regular expressions in order for that test case to be run')
    parser.add_argument('-x', '--exclude', action='append', metavar='regex', help='Negative line filter: a line (more precisely, workingdir+SPACE+args) must NOT match any of the regular expressions in order for that test case to be run')
    parser.add_argument('-t', '--threads', type=int, default=defaultNumThreads, help='number of parallel threads (default: number of CPUs, currently '+str(defaultNumThreads)+')')
    parser.add_argument('-r', '--repeat', type=int, default=1, help='number of repeating each test (default: 1)')
    parser.add_argument('-e', '--executable', default='inet', help='Determines which binary to execute (e.g. opp_run_dbg, opp_run_release) if the command column in the CSV file does not specify one.')
    parser.add_argument('-C', '--directory', help='Change to DIRECTORY before executing the tests. Working dirs in the CSV files are relative to this.')
    parser.add_argument('-d', '--debug', action='store_true', help='Run debug executables: use the debug version of the executable (appends _dbg to the executable name)')
    parser.add_argument('-s', '--release', action='store_true', help='Run release executables: use the release version of the executable (appends _release to the executable name)')
    parser.add_argument('-q', '--disable_outfile', dest='writeOutfile', action='store_false', help='disable the writing standard output of simulations to result directory')
    parser.add_argument('-c', '--calculator', default=defaultFingerprintCalculator, help='Set the fingerprintcalculator-class. Empty value means the original OmNET++ fingerprint calculator')
    parser.add_argument('-a', '--oppargs', action='append', metavar='oppargs', nargs=argparse.REMAINDER, help='extra opp_run arguments until the end of the line')
    parser.add_argument('-n', type=int, default=1, help='Split the selected test cases into n sets, and only run one of these sets')
    parser.add_argument('-i', type=int, default=0, help='Which set of test cases to run [0..n-1]')
    parser.add_argument('-l', '--logfile', default = "fingerprinttest.out", metavar='filename', help='name of the logfile')
    args = parser.parse_args()

    logFile = args.logfile

    # create / reset logFile
    FILE = open(logFile, "w")
    FILE.close()

    if not args.testspecfiles:
        args.testspecfiles = glob.glob('*.csv')

    if args.calculator:
        extraOppRunArgs += " --fingerprintcalculator-class=" + args.calculator

    if args.oppargs:
        for oppArgList in args.oppargs:
            for oppArg in oppArgList:
                extraOppRunArgs += " " + oppArg

    if args.executable:
        executable = args.executable

    if args.directory:
        rootDir = os.path.abspath(args.directory)

    debug = args.debug
    release = args.release
    writeOutfile = args.writeOutfile

    generator = FingerprintTestCaseGenerator()
    testcases = generator.generateFromCSV(args.testspecfiles, args.fpfilter, args.excludefpfilter, args.match, args.exclude, args.repeat)

    testcases = testcases[args.i::args.n]

    testSuite = ThreadedTestSuite()
    testSuite.addTests(testcases)
    testSuite.thread_count = args.threads
    testSuite.repeat = args.repeat

    testRunner = unittest.TextTestRunner(stream=sys.stdout, verbosity=9, resultclass=SimulationTextTestResult)

    testRunner.run(testSuite)

    print()

    generator.writeUpdatedCSVFiles()
    generator.writeErrorCSVFiles()
    generator.writeFailedCSVFiles()

    print("Log has been saved to %s" % logFile)

    if exitCode == 0:
        print("Test results equals to expected results")
    else:
        print("Test results differ from expected results")
    exit(exitCode)
