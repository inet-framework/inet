//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispServerEntry.h"

#include <sstream>

namespace inet {
namespace lisp {

LispServerEntry::LispServerEntry(std::string nipv)
{
    address = !nipv.empty() ? L3Address(nipv.c_str()) : L3Address();
}

LispServerEntry::LispServerEntry(std::string nipv, std::string nkey, bool proxy, bool notify, bool quick) :
    key(nkey), proxyReply(proxy), mapNotify(notify), quickRegistration(quick), lastTime(simTime())
{
    address = !nipv.empty() ? L3Address(nipv.c_str()) : L3Address();
}

bool LispServerEntry::operator<(const LispServerEntry& other) const
{
    if (address.getType() != L3Address::IPv6 && other.address.getType() == L3Address::IPv6)
        return true;
    else if (address.getType() == L3Address::IPv6 && other.address.getType() != L3Address::IPv6)
        return false;
    if (address < other.address) return true;
    if (lastTime < other.lastTime) return true;
    return false;
}

bool LispServerEntry::operator==(const LispServerEntry& other) const
{
    return address == other.address && key == other.key && proxyReply == other.proxyReply
           && mapNotify == other.mapNotify && quickRegistration == other.quickRegistration
           && lastTime == other.lastTime;
}

std::string LispServerEntry::str() const
{
    std::stringstream os;
    os << address.str();
    if (!key.empty())
        os << ", key: \"" << key << "\"";
    if (lastTime != SIMTIME_ZERO)
        os << ", last at: " << lastTime;
    if (proxyReply)
        os << ", proxy-reply";
    if (mapNotify)
        os << ", map-notify";
    if (quickRegistration)
        os << ", quick-registration";
    return os.str();
}

std::ostream& operator<<(std::ostream& os, const LispServerEntry& entry)
{
    return os << entry.str();
}

} // namespace lisp
} // namespace inet
