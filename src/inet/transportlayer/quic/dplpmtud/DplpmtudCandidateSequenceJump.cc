//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "DplpmtudCandidateSequenceJump.h"

#include <sstream>
#include <omnetpp/clog.h>
#include <omnetpp/cexception.h>

namespace inet {
namespace quic {

using namespace std;
using namespace omnetpp;

string print2(std::vector<int> tree) {
    stringstream output;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        output << *it << " ";
    }
    return output.str();
}

DplpmtudCandidateSequenceJump::DplpmtudCandidateSequenceJump(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequence(minPmtu, maxPmtu, stepSize) {
    downward = true;

    // add initial candidates to test
    // respecting the range given by [minPmtu, maxPmtu] add these candidates:
    // 1300, 1400, 1420, 1440, 1460, 1480, 1500, 1520, 4000, 6500, 9000, 12500, 34000, 46500, 59000
    // and add minPmtu and maxPmtu
    int candidate;
    candidates.push_back(minPmtu);
    for (candidate = 1300; candidate <= 1400 && candidate < maxPmtu; candidate += 100) {
        if (candidate > minPmtu) {
            candidates.push_back(candidate);
        }
    }
    for (candidate = 1420; candidate <= 1520 && candidate < maxPmtu; candidate += 20) {
        if (candidate > minPmtu) {
            candidates.push_back(candidate);
        }
    }
    for (candidate = 4000; candidate <= 9000 && candidate < maxPmtu; candidate += 2500) {
        if (candidate > minPmtu) {
            candidates.push_back(candidate);
        }
    }
    for (candidate = 21500; candidate <= 59000 && candidate < maxPmtu; candidate += 12500) {
        if (candidate > minPmtu) {
            candidates.push_back(candidate);
        }
    }
    candidates.push_back(maxPmtu);
    currentIt = candidates.end();

}
DplpmtudCandidateSequenceJump::~DplpmtudCandidateSequenceJump() { }

/**
 * Remove all candidates larger than ptbMtu and add ptbMtu.
 */
void DplpmtudCandidateSequenceJump::ptbReceived(int ptbMtu) {
    DplpmtudCandidateSequence::ptbReceived(ptbMtu);
    std::vector<int>::iterator it;
    for(it = candidates.begin(); it != candidates.end() && *it < ptbMtu; ++it);

    candidates.erase(it, candidates.end());
    candidates.push_back(ptbMtu);
}

/**
 * \param value Value in candidates to get an iterator for.
 * \return Iterator pointing to the element in candidates with the given value.
 */
std::vector<int>::iterator DplpmtudCandidateSequenceJump::getIterator(int value) {
    for(std::vector<int>::iterator it = candidates.begin(); it != candidates.end(); ++it) {
        if (*it == value) {
            return it;
        }
    }
    throw cRuntimeError("DplpmtudSearchAlgorithmJump: Could not find iterator for value");
}

/**
 * Add further candidates after the element the given iterator points to.
 * \param after An iterator pointing to the element in candidates after that new candidates are to be added.
 */
void DplpmtudCandidateSequenceJump::addCandidates(std::vector<int>::iterator &after) {
    int lower = *after;
    int upper = *(after+1);
    int d = upper - lower;
    if (d <= 4) {
        //throw omnetpp::cRuntimeError("DplpmtudSearchAlgorithmJump::addCandidates: adding candidates is not possible as the distance does not exceed 4.");
        EV_WARN << "DplpmtudSearchAlgorithmJump::addCandidates: adding candidates is not possible as the distance does not exceed 4." << endl;
        return;
    }
    // calculate the distance for new candidates based on the old distance d
    int newD;
    if (d%5 != 0 || (d/5)%4 != 0) {
        for (newD=4; newD*5 < d; newD*=5);
    } else {
        newD = d/5;
    }
    // add new candidates in descending order
    std::vector<int>::iterator insertBefore = (after+1);
    for (int newCandidate = upper-newD; newCandidate > lower; newCandidate -= newD) {
        insertBefore = candidates.insert(insertBefore, newCandidate);
    }
    // refresh the iterator given by the caller
    after = insertBefore-1;
}


int DplpmtudCandidateSequenceJump::getNextCandidate(int probeSizeLimit) {
    EV_DEBUG << "DplpmtudSearchAlgorithmJump::getNextCandidate begin candidates: " << print2(candidates) << ", currentIt: " << *currentIt << endl;
    if (downward) {
        if (currentIt == candidates.begin()
         || *(currentIt-1) > probeSizeLimit) {
            return 0;
        }
        currentIt--;
    } else {
        //do {
            //EV_DEBUG << "candidate " << *currentIt << " already used" << endl;
            if ((currentIt+1) == candidates.end()
             || *(currentIt+1) >= smallestExpiredProbeSize
             || *(currentIt+1) > probeSizeLimit) {
                EV_DEBUG << "DplpmtudSearchAlgorithmJump::getNextCandidate could not select candidate, it would be: " << ((currentIt+1) == candidates.end() ? -1 : *(currentIt+1)) << endl;
                return 0;
            }
            currentIt++;
        //} while (usedCandidates.find(*currentIt) != usedCandidates.end());
    }

    //usedCandidates.insert(*currentIt);
    EV_DEBUG << "DplpmtudSearchAlgorithmJump::getNextCandidate end candidate: " << print2(candidates) << ", currentIt: " << *currentIt << endl;
    return *currentIt;
}

void DplpmtudCandidateSequenceJump::testSucceeded(int succeededCandidate) {
    EV_DEBUG << "DplpmtudSearchAlgorithmJump::testSucceeded(" << succeededCandidate << ") begin candidate: " << print2(candidates) << ", currentIt: " << *currentIt << endl;
    if (downward) {
        downward = false;
        auto it = getIterator(succeededCandidate);
        if (it+1 != candidates.end()) {
            addCandidates(it);
            currentIt = it;
            EV_DEBUG << "next currentIt: " << *(currentIt+1) << ", next it: " << *(it+1) << endl;
        }
    }
    EV_DEBUG << "DplpmtudSearchAlgorithmJump::testSucceeded(" << succeededCandidate << ") end candidate: " << print2(candidates) << ", currentIt: " << *currentIt << endl;
}


void DplpmtudCandidateSequenceJump::testFailed(int failedCandidate) {
    EV_DEBUG << "DplpmtudSearchAlgorithmJump::testFailed(" << failedCandidate << ") begin candidate: " << print2(candidates) << ", currentIt: " << *currentIt << endl;
    maxPmtu = failedCandidate-stepSize;
    auto deleteStartingFrom = candidates.begin();
    for (; deleteStartingFrom != candidates.end() && *deleteStartingFrom < maxPmtu; ++deleteStartingFrom);
    candidates.erase(deleteStartingFrom, candidates.end());
    candidates.push_back(maxPmtu);
    EV_DEBUG << "DplpmtudSearchAlgorithmJump::testFailed(" << failedCandidate << ") end candidate: " << print2(candidates) << ", currentIt: " << *currentIt << endl;
}

bool DplpmtudCandidateSequenceJump::repeatOnTimeout(int size) {
    auto it = getIterator(size);
    if (it == candidates.begin()) {
        EV_DEBUG << "DplpmtudSearchAlgorithmJump::repeatOnTimeout(" << size << ") returns true, because it is the smallest candidate, currentIt: " << *currentIt << endl;
        return true;
    }
    if (!downward) {
        if (currentIt+1 == it && (*it - *(it-1)) <= 4) {
            // this is the next candidate and we cannot add more candidates between the current one and the next one.
            EV_DEBUG << "DplpmtudSearchAlgorithmJump::repeatOnTimeout(" << size << ") returns true, because it is the next candidate and we cannot add more candidates between the current one and the next one, currentIt: " << *currentIt << endl;
            return true;
        }
//        if (it == currentIt && (*currentIt - *(currentIt-1)) <= 4) {
//            // this is the one we have currently selected and we cannot add more candidates below this one.
//            EV_DEBUG << "DplpmtudSearchAlgorithmJump::repeatOnTimeout(" << size << ") returns true, because it is the current candidate and we cannot select smaller ones, currentIt: " << *currentIt << endl;
//            return true;
//        }
//        if (it == currentIt+1) {
//            // this is the one we would select next.
//            EV_DEBUG << "DplpmtudSearchAlgorithmJump::repeatOnTimeout(" << size << ") returns true, because it is the candidate we would select next, currentIt: " << *currentIt << endl;
//            return true;
//        }
        //&& (*it - *(it-1)) <= 4 && *(it-1) < *currentIt) { //usedCandidates.find(*(it-1)) != usedCandidates.end()) {
    }

    EV_DEBUG << "DplpmtudSearchAlgorithmJump::repeatOnTimeout(" << size << ") returns false, currentIt: " << *currentIt << endl;
    return false;
}

void DplpmtudCandidateSequenceJump::smallestProbeTimedOut(int size) {
    if (!downward) {
        auto it = getIterator(size);
        if (it != candidates.begin()) {
            it--;
            addCandidates(it);
            currentIt = it;
        }
    }
}

} /* namespace quic */
} /* namespace inet */
