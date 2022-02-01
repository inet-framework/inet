//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACETOKEN_H
#define __INET_INTERFACETOKEN_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * An "interface token" as defined in RFC 1971 (Ipv6 Stateless Autoconfiguration).
 * This class supports tokens of length 1..64-bits. An interface token needs
 * to be provided by L2 modules in order to be able to form Ipv6 link local
 * addresses.
 */
class INET_API InterfaceToken
{
  private:
    uint32_t _normal, _low;
    short _len; // in bits, 1..64

  private:
    void copy(const InterfaceToken& t) { _normal = t._normal; _low = t._low; _len = t._len; }

  public:
    InterfaceToken() { _normal = _low = _len = 0; }
    InterfaceToken(uint32_t low, uint32_t normal, int len) { _normal = normal; _low = low; _len = len; }
    InterfaceToken(const InterfaceToken& t) { copy(t); }
    InterfaceToken& operator=(const InterfaceToken& t) { copy(t); return *this; }
    int length() const { return _len; }
    uint32_t low() const { return _low; }
    uint32_t normal() const { return _normal; }
};

} // namespace inet

#endif

