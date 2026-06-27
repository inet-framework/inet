//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispMapEntry.h"

#include <sstream>

namespace inet {
namespace lisp {

bool LispMapEntry::isLocatorExisting(const L3Address& address) const
{
    for (const auto& rloc : RLOCs)
        if (rloc.getRlocAddr() == address)
            return true;
    return false;
}

LispRlocator *LispMapEntry::getLocator(const L3Address& address)
{
    for (auto& rloc : RLOCs)
        if (rloc.getRlocAddr() == address)
            return &rloc;
    return nullptr;
}

void LispMapEntry::addLocator(LispRlocator& entry)
{
    RLOCs.push_back(entry);
    RLOCs.sort();
}

void LispMapEntry::removeLocator(L3Address& address)
{
    LispRlocator *rloc = getLocator(address);
    if (rloc)
        RLOCs.remove(*rloc);
}

bool LispMapEntry::operator==(const LispMapEntry& other) const
{
    return EID == other.EID && ttl == other.ttl && expiry == other.expiry && RLOCs == other.RLOCs;
}

bool LispMapEntry::operator<(const LispMapEntry& other) const
{
    if (EID < other.EID) return true;
    if (expiry < other.expiry) return true;
    if (Action < other.Action) return true;
    if (RLOCs < other.RLOCs) return true;
    return false;
}

std::string LispMapEntry::getMapStateString() const
{
    switch (getMapState()) {
        case INCOMPLETE: return "incomplete";
        case COMPLETE: return "complete";
        default: return "UNKNOWN";
    }
}

std::string LispMapEntry::getActionString() const
{
    switch (Action) {
        case LispCommon::DROP: return "drop";
        case LispCommon::SEND_MAP_REQUEST: return "send-map-request";
        case LispCommon::NATIVELY_FORWARD: return "natively-forward";
        case LispCommon::NO_ACTION:
        default: return "no-action";
    }
}

LispRlocator *LispMapEntry::getBestUnicastLocator(cRNG *rng)
{
    // collect the UP locators with the lowest (best) priority value
    std::vector<LispRlocator *> best;
    int bestPriority = 256;
    for (auto& rloc : RLOCs) {
        if (rloc.getState() != LispRlocator::UP)
            continue;
        if (rloc.getPriority() < bestPriority) {
            best.clear();
            best.push_back(&rloc);
            bestPriority = rloc.getPriority();
        }
        else if (rloc.getPriority() == bestPriority)
            best.push_back(&rloc);
    }
    if (best.empty())
        return nullptr;
    // pick one weighted by its weight
    double dice = rng->doubleRandIncl1();
    double accumulated = 0;
    for (auto *rloc : best) {
        accumulated += rloc->getWeight() / 100.0;
        if (dice <= accumulated)
            return rloc;
    }
    return best.front();
}

std::string LispMapEntry::str() const
{
    std::stringstream os;
    os << EID;
    os << ", expires: ";
    if (expiry == SIMTIME_ZERO)
        os << " never";
    else
        os << ttl << "min (" << expiry << ")";
    os << ", state: " << getMapStateString();
    os << ", action: " << getActionString();
    for (const auto& rloc : RLOCs)
        os << "\n   " << rloc.str();
    return os.str();
}

std::ostream& operator<<(std::ostream& os, const LispMapEntry& me)
{
    return os << me.str();
}

std::ostream& operator<<(std::ostream& os, const Locators& rlocs)
{
    for (const auto& rloc : rlocs)
        os << rloc.str() << "\n";
    return os;
}

} // namespace lisp
} // namespace inet
