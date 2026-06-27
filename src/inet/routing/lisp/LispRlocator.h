//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPRLOCATOR_H
#define __INET_LISPRLOCATOR_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/lisp/LispCommon.h"

namespace inet {
namespace lisp {

/**
 * A Routing LOCator (RLOC): a reachable locator address with its priority,
 * weight, multicast priority/weight, reachability state and local flag.
 */
class INET_API LispRlocator
{
  public:
    enum LocatorState { DOWN, ADMIN_DOWN, UP };

  private:
    L3Address rloc;
    LocatorState state = DOWN;
    unsigned char priority = DEFAULT_PRIORITY_VAL;
    unsigned char weight = DEFAULT_WEIGHT_VAL;
    unsigned char mpriority = DEFAULT_MPRIORITY_VAL;
    unsigned char mweight = DEFAULT_MWEIGHT_VAL;
    bool local = false;

  public:
    LispRlocator();
    explicit LispRlocator(const char *addr);
    LispRlocator(const char *addr, const char *prio, const char *wei, bool loca);

    bool operator==(const LispRlocator& other) const;
    bool operator<(const LispRlocator& other) const;

    std::string str() const;

    const L3Address& getRlocAddr() const;
    void setRlocAddr(const L3Address& rloc);
    LocatorState getState() const;
    std::string getStateString() const;
    void setState(LocatorState state);
    unsigned char getPriority() const;
    void setPriority(unsigned char priority);
    unsigned char getWeight() const;
    void setWeight(unsigned char weight);
    unsigned char getMpriority() const;
    void setMpriority(unsigned char mpriority);
    unsigned char getMweight() const;
    void setMweight(unsigned char mweight);
    bool isLocal() const;
    void setLocal(bool local);
    LispCommon::Afi getRlocAfi() const;

    void updateRlocator(const LispRlocator& rloc);
};

std::ostream& operator<<(std::ostream& os, const LispRlocator& locator);

} // namespace lisp
} // namespace inet

#endif
