//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispSiteRecord.h"

#include <sstream>

namespace inet {
namespace lisp {

std::string LispSiteRecord::str() const
{
    std::stringstream os;
    os << "ETR " << ServerEntry.str() << "\n";
    os << LispMapStorageBase::str();
    return os.str();
}

bool LispSiteRecord::operator==(const LispSiteRecord& other) const
{
    return ServerEntry == other.ServerEntry && MappingStorage == other.MappingStorage;
}

bool LispSiteRecord::operator<(const LispSiteRecord& other) const
{
    if (ServerEntry < other.ServerEntry) return true;
    if (MappingStorage < other.MappingStorage) return true;
    return false;
}

std::ostream& operator<<(std::ostream& os, const LispSiteRecord& sr)
{
    return os << sr.str();
}

} // namespace lisp
} // namespace inet
