//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudProbes.h"

#include <omnetpp/cexception.h>

namespace inet {
namespace quic {

DplpmtudProbes::DplpmtudProbes() { }

DplpmtudProbes::~DplpmtudProbes() {
    for (auto it=this->begin(); it != this->end(); ++it) {
        delete it->second;
    }
}

void DplpmtudProbes::add(DplpmtudProbe *probe) {
    if (getBySize(probe->getSize()) != nullptr) {
        throw omnetpp::cRuntimeError("DPLPMTUD: add probe already outstanding");
    }
    this->insert( {probe->getSize(), probe} );
}

void DplpmtudProbes::removeAll() {
    for (auto it=this->begin(); it != this->end(); it = this->erase(it)) {
        delete it->second;
    }
}

void DplpmtudProbes::removeAllEqualOrSmaller(int size) {
    for (auto it=this->begin(); it != this->end() && it->first <= size; it = this->erase(it)) {
        delete it->second;
    }
}

void DplpmtudProbes::removeAllLarger(int size) {
    auto it=this->begin();
    for (; it != this->end() && it->first <= size; ++it);
    for (; it != this->end(); it = this->erase(it)) {
        delete it->second;
    }
}

void DplpmtudProbes::removeAllEqualOrLarger(int size) {
    auto it=this->begin();
    for (; it != this->end() && it->first < size; ++it);
    for (; it != this->end(); it = this->erase(it)) {
        delete it->second;
    }
}

DplpmtudProbe *DplpmtudProbes::getBySize(int size) {
    auto it = this->find(size);
    if (it == this->end()) {
        return nullptr;
    }
    return it->second;
}

DplpmtudProbe *DplpmtudProbes::getSmallest() {
    if (this->size() == 0) {
        return nullptr;
    }
    return this->begin()->second;
}

DplpmtudProbe *DplpmtudProbes::getSmallestLost() {
    for (auto it=this->begin(); it != this->end(); ++it) {
        DplpmtudProbe *probe = it->second;
        if (probe->isLost()) {
            return probe;
        }
    }
    return nullptr;
}

bool DplpmtudProbes::containsProbesNotLost() {
    for (auto it=this->begin(); it != this->end(); ++it) {
        DplpmtudProbe *probe = it->second;
        if (!probe->isLost()) {
            return true;
        }
    }
    return false;
}

std::string DplpmtudProbes::str() {
    std::stringstream str;
    bool first = true;
    str << "[";
    for (auto it=this->begin(); it != this->end(); ++it) {
        DplpmtudProbe *probe = it->second;
        if (first) {
            first = false;
        } else {
            str << ", ";
        }
        str << probe->str();
    }
    str << "]";
    return str.str();
}

} /* namespace quic */
} /* namespace inet */
