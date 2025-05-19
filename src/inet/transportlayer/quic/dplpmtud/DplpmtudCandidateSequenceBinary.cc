//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudCandidateSequenceBinary.h"

#include <sstream>
#include <omnetpp/clog.h>

namespace inet {
namespace quic {

using namespace std;
using namespace omnetpp;

string print(std::vector<int> tree) {
    stringstream output;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        output << *it << " ";
    }
    return output.str();
}

DplpmtudCandidateSequenceBinary::DplpmtudCandidateSequenceBinary(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequence(minPmtu, maxPmtu, stepSize) {
    gotAck = false;

    candidatesTree.push_back(minPmtu);
    candidatesTree.push_back(maxPmtu);
    EV_DEBUG << "DplpmtudSearchAlgorithmBinary candidatesTree: " << print(candidatesTree) << endl;
}
DplpmtudCandidateSequenceBinary::~DplpmtudCandidateSequenceBinary() { }

int DplpmtudCandidateSequenceBinary::calculateNextValue() {
    std::vector<int>::iterator lastMinusOneIt;
    int next;

    do {
        // find the next value to calculate, that is the first -1 from right
        for (lastMinusOneIt=candidatesTree.end()-2; lastMinusOneIt > candidatesTree.begin(); --lastMinusOneIt) {
            if (*lastMinusOneIt == -1) {
                break;
            }
        }

        if (*lastMinusOneIt != -1) {
            // no -1 found, add a new level in the tree
            // add a -1 between each two elements
            for (auto it=candidatesTree.begin()+1; it!=candidatesTree.end(); ++it) {
                if ((*it - *(it-1)) >= 2*stepSize
                || ((*it - *(it-1)) >= stepSize && ((it-1) == candidatesTree.begin() || it == candidatesTree.end()-1))) {

                    it = candidatesTree.insert(it, -1);
                    lastMinusOneIt = it;
                    ++it;
                }
            }
        }

        if (*lastMinusOneIt != -1) {
            // no new candidates available
            return 0;
        }

        int min = *(lastMinusOneIt-1);
        if ((lastMinusOneIt-1) != candidatesTree.begin()) {
            min += stepSize;
        }
        int max = *(lastMinusOneIt+1);
        if ((lastMinusOneIt+1) != (candidatesTree.end()-1)) {
            max -= stepSize;
        }
        next = std::ceil( ((double)(max - min)) / (stepSize * 2) )  * stepSize + min;
        *lastMinusOneIt = next;

        if (next >= smallestExpiredProbeSize) {
            retainedCandidates.push_back(next);
        }
    } while (next >= smallestExpiredProbeSize);

    EV_DEBUG << "DplpmtudSearchAlgorithmBinary::calculateNextValue candidatesTree: " << print(candidatesTree) << endl;
    return next;
}

void DplpmtudCandidateSequenceBinary::testSucceeded(int succeededCandidate) {
    // delete candidates equal or smaller than succeededCandidate
    for (auto it = candidatesTree.begin(); it != candidatesTree.end()-1 && *it <= succeededCandidate; it = candidatesTree.erase(it));

    // add new lower bound
    candidatesTree.insert(candidatesTree.begin(), succeededCandidate+stepSize);

    EV_DEBUG << "DplpmtudSearchAlgorithmBinary::testSucceeded(" << succeededCandidate << ") candidatesTree: " << print(candidatesTree) << endl;

    for (auto it = retainedCandidates.begin(); it != retainedCandidates.end();) {
        if (*it <= succeededCandidate) {
            it = retainedCandidates.erase(it);
        } else {
            ++it;
        }
    }
}
void DplpmtudCandidateSequenceBinary::testFailed(int failedCandidate) {
    // delete all candidates equal or larger than failedCandidate
    for (auto it = candidatesTree.end()-1; it != candidatesTree.begin() && (*it >= failedCandidate || *it == -1); it = candidatesTree.erase(it)-1);

    // add new upper bound
    candidatesTree.insert(candidatesTree.end(), failedCandidate-stepSize);

    EV_DEBUG << "DplpmtudSearchAlgorithmBinary::testFailed(" << failedCandidate << ")  candidatesTree: " << print(candidatesTree) << endl;

    for (auto it = retainedCandidates.begin(); it != retainedCandidates.end();) {
        if (*it >= failedCandidate) {
            it = retainedCandidates.erase(it);
        } else {
            ++it;
        }
    }
}

void DplpmtudCandidateSequenceBinary::ptbReceived(int ptbMtu) {
    DplpmtudCandidateSequence::ptbReceived(ptbMtu);
    // delete all candidates equal or larger  than ptbMtu
    for (auto it = candidatesTree.end()-1; it != candidatesTree.begin() && (*it >= ptbMtu || *it == -1); it = candidatesTree.erase(it)-1);

    // add new upper bound
    candidatesTree.insert(candidatesTree.end(), ptbMtu);

    EV_DEBUG << "DplpmtudSearchAlgorithmBinary::ptbReceived(" << ptbMtu << ")  candidatesTree: " << print(candidatesTree) << endl;

    for (auto it = retainedCandidates.begin(); it != retainedCandidates.end();) {
        if (*it > ptbMtu) {
            it = retainedCandidates.erase(it);
        } else {
            ++it;
        }
    }
}

int DplpmtudCandidateSequenceBinary::getNextCandidate(int probeSizeLimit) {
    if (candidatesTree.size() == 2 && candidatesTree[0] > candidatesTree[1]) {
        return 0;
    }
    EV_DEBUG << "DplpmtudSearchAlgorithmBinary::getNextCandidate check retainedCandidates with smallestExpiredProbeSize=" << smallestExpiredProbeSize << endl;
    for (auto it=retainedCandidates.begin(); it != retainedCandidates.end(); ++it) {
        if (*it < smallestExpiredProbeSize) {
            int next = *it;
            if (next > probeSizeLimit) {
                return 0;
            }
            retainedCandidates.erase(it);
            return next;
        }
    }

    int next = calculateNextValue();
    if (next > probeSizeLimit) {
        retainedCandidates.push_back(next);
        return 0;
    }
    return next;
}

bool DplpmtudCandidateSequenceBinary::repeatOnTimeout(int size) {
    return (candidatesTree[0] >= size);
}

} /* namespace quic */
} /* namespace inet */
