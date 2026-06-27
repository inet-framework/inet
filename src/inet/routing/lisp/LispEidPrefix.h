//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPEIDPREFIX_H
#define __INET_LISPEIDPREFIX_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/lisp/LispCommon.h"

namespace inet {
namespace lisp {

/**
 * An Endpoint IDentifier (EID) prefix: an IPv4 or IPv6 address together with a
 * prefix length.
 */
class INET_API LispEidPrefix
{
  private:
    L3Address eidAddr;
    L3Address eidNetwork;
    unsigned char eidLen = DEFAULT_EIDLENGTH_VAL;

  public:
    LispEidPrefix();
    LispEidPrefix(const char *address, const char *length);
    LispEidPrefix(L3Address address, unsigned char length);

    bool operator==(const LispEidPrefix& other) const;
    bool operator<(const LispEidPrefix& other) const;

    std::string str() const;

    const L3Address& getEidAddr() const;
    void setEidAddr(const L3Address& eid);
    unsigned char getEidLength() const;
    void setEidLength(unsigned char eidLen);
    LispCommon::Afi getEidAfi() const;

    /** True if this prefix is contained in (is a more-specific of) the given coarser prefix. */
    bool isComponentOf(const LispEidPrefix& coarserEid) const;
};

std::ostream& operator<<(std::ostream& os, const LispEidPrefix& ep);

} // namespace lisp
} // namespace inet

#endif
