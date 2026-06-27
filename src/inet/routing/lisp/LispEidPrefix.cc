//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispEidPrefix.h"

#include <sstream>

namespace inet {
namespace lisp {

LispEidPrefix::LispEidPrefix()
{
    eidAddr = Ipv4Address::UNSPECIFIED_ADDRESS;
    eidLen = DEFAULT_EIDLENGTH_VAL;
    eidNetwork = LispCommon::getNetworkAddress(eidAddr, eidLen);
}

LispEidPrefix::LispEidPrefix(const char *address, const char *length)
{
    eidAddr = L3Address(address);
    eidLen = (unsigned char)atoi(length);
    eidNetwork = LispCommon::getNetworkAddress(eidAddr, eidLen);
}

LispEidPrefix::LispEidPrefix(L3Address address, unsigned char length)
{
    eidAddr = address;
    eidLen = length;
    eidNetwork = LispCommon::getNetworkAddress(eidAddr, eidLen);
}

const L3Address& LispEidPrefix::getEidAddr() const { return eidNetwork; }

void LispEidPrefix::setEidAddr(const L3Address& eid) { this->eidAddr = eid; }

unsigned char LispEidPrefix::getEidLength() const { return eidLen; }

void LispEidPrefix::setEidLength(unsigned char eidLen) { this->eidLen = eidLen; }

LispCommon::Afi LispEidPrefix::getEidAfi() const
{
    return (eidAddr.getType() == L3Address::IPv6) ? LispCommon::AFI_IPV6 : LispCommon::AFI_IPV4;
}

std::string LispEidPrefix::str() const
{
    std::stringstream os;
    os << eidAddr << "/" << short(eidLen);
    return os.str();
}

bool LispEidPrefix::operator==(const LispEidPrefix& other) const
{
    return eidAddr == other.eidAddr && eidLen == other.eidLen;
}

bool LispEidPrefix::operator<(const LispEidPrefix& other) const
{
    if (getEidAfi() < other.getEidAfi()) return true;
    if (eidAddr < other.eidAddr) return true;
    if (eidLen < other.eidLen) return true;
    return false;
}

bool LispEidPrefix::isComponentOf(const LispEidPrefix& coarserEid) const
{
    // a component must have a finer (longer) mask and the same address family
    if (eidLen > coarserEid.eidLen || getEidAfi() != coarserEid.getEidAfi())
        return false;
    int result = LispCommon::doPrefixMatch(eidAddr, coarserEid.eidAddr);
    return result == -1 || result >= coarserEid.eidLen;
}

std::ostream& operator<<(std::ostream& os, const LispEidPrefix& ep)
{
    return os << ep.str();
}

} // namespace lisp
} // namespace inet
