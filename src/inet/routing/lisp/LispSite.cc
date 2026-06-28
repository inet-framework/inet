//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispSite.h"

#include <sstream>

namespace inet {
namespace lisp {

std::string LispSite::str() const
{
    std::stringstream os;
    os << name << ", key: \"" << key << "\"\nMaintained EIDs>\n";
    for (const auto& entry : MappingStorage)
        os << entry.getEidPrefix() << "\n";
    if (ETRs.size()) {
        os << "\nRegistered ETRs>\n";
        for (const auto& etr : ETRs)
            os << etr.str() << "\n";
    }
    return os.str();
}

bool LispSite::operator==(const LispSite& other) const
{
    return name == other.name && key == other.key && ETRs == other.ETRs;
}

LispSiteRecord *LispSite::findRecordByAddress(L3Address& address)
{
    for (auto& etr : ETRs)
        if (etr.getServerEntry().getAddress() == address)
            return &etr;
    return nullptr;
}

Etrs LispSite::findAllRecordsByEid(const L3Address& address)
{
    Etrs result;
    for (auto& etr : ETRs)
        if (etr.lookupMapEntry(address))
            result.push_back(etr);
    return result;
}

bool LispSite::isEidMaintained(const L3Address& address)
{
    for (const auto& entry : MappingStorage) {
        // skip non-comparable address families
        if ((address.getType() == L3Address::IPv6) != (entry.getEidPrefix().getEidAddr().getType() == L3Address::IPv6))
            continue;
        int commonbits = LispCommon::doPrefixMatch(entry.getEidPrefix().getEidAddr(), address);
        if (commonbits == -1 || commonbits >= entry.getEidPrefix().getEidLength())
            return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const LispSite& si)
{
    return os << si.str();
}

} // namespace lisp
} // namespace inet
