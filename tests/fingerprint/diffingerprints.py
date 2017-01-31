#!/usr/bin/env python3
import sys
import re

def computeUniqueFingerprints(fingerprints):
    i = 0
    uniqueFingerprints = list()
    while i < len(fingerprints):
        fingerprint = fingerprints[i]
        j = i
        while (j < len(fingerprints)) and (fingerprint == fingerprints[j]):
            j = j + 1
        uniqueFingerprints.append([j-1, fingerprint])
        i = j
    return uniqueFingerprints

def readFingerprints1(eventlogFile):
    "RegExp version"
    regExp = re.compile("^E # (\d+) t .+ m \d+ ce \d+ msg \d+ f ([0-z]{4}-[0-z]{4})")
    fingerprints = list()
    for line in eventlogFile:
        match = regExp.match(line)
        if match:
            fingerprint = match.group(2)
            fingerprints.append(fingerprint)
    return fingerprints

def isNewEvent(line):
    return len(line) >= 4 and line[0:4] == "E # "

def getFingerprint(line):
    n = len(line) - 1
    return line[(n-14):(n-5)]

def readFingerprints2(eventlogFile):
    fingerprints = list()
    eventlogFile.readline() # skip the first line
    for line in eventlogFile:
        if isNewEvent(line):
            fingerprint = getFingerprint(line)
            fingerprints.append(fingerprint)
    return fingerprints

def diffingerprint(elog1Fingerprints, elog2Fingerprints):
    minSize = min(len(elog1Fingerprints), len(elog2Fingerprints)) - 1
    for i in range(0,minSize):
        elog1Fingerprint = elog1Fingerprints[i]
        elog2Fingerprint = elog2Fingerprints[i]
        if elog1Fingerprint[1] != elog2Fingerprint[1]:
            return "Diff elog1: " + str(elog1Fingerprint[0]) + " fingerprint = " + elog1Fingerprint[1] + ", elog2: " + str(elog2Fingerprint[0]) + " fingerprint = " + elog2Fingerprint[1]
    return "No differences"


elog1FilePath = sys.argv[1]
elog2FilePath = sys.argv[2]

elog1File = open(elog1FilePath)
elog2File = open(elog2FilePath)
uniqueElog1Fingerprints = computeUniqueFingerprints(readFingerprints1(elog1File))
uniqueElog2Fingerprints = computeUniqueFingerprints(readFingerprints1(elog2File))
print(diffingerprint(uniqueElog1Fingerprints, uniqueElog2Fingerprints))

elog1File.close()
elog2File.close()
