//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPMAPENTRY_H
#define __INET_LISPMAPENTRY_H

#include <list>

#include "inet/routing/lisp/LispCommon.h"
#include "inet/routing/lisp/LispEidPrefix.h"
#include "inet/routing/lisp/LispRlocator.h"

namespace inet {
namespace lisp {

typedef std::list<LispRlocator> Locators;
typedef Locators::iterator LocatorItem;
typedef Locators::const_iterator LocatorCItem;

/**
 * A single EID-to-RLOC mapping: an EID prefix, its set of locators (RLOCs), the
 * cache TTL/expiry and the map-cache action.
 */
class INET_API LispMapEntry
{
  public:
    enum MapState { INCOMPLETE, COMPLETE };

  protected:
    LispEidPrefix EID;
    Locators RLOCs;
    unsigned int ttl = 0;
    simtime_t expiry = SIMTIME_ZERO;
    LispCommon::EAct Action = LispCommon::NO_ACTION;

  public:
    LispMapEntry() {}
    explicit LispMapEntry(LispEidPrefix neid) : EID(neid) {}
    virtual ~LispMapEntry() {}

    bool operator==(const LispMapEntry& other) const;
    bool operator<(const LispMapEntry& other) const;

    const LispEidPrefix& getEidPrefix() const { return EID; }
    void setEidPrefix(const LispEidPrefix& eidPrefix) { EID = eidPrefix; }
    const simtime_t& getExpiry() const { return expiry; }
    void setExpiry(const simtime_t& expiry) { this->expiry = expiry; }
    MapState getMapState() const { return RLOCs.size() ? COMPLETE : INCOMPLETE; }
    std::string getMapStateString() const;
    const Locators& getRlocs() const { return RLOCs; }
    Locators& getRlocs() { return RLOCs; }
    void setRlocs(const Locators& rlocs) { RLOCs = rlocs; }
    LispCommon::EAct getAction() const { return Action; }
    std::string getActionString() const;
    void setAction(const LispCommon::EAct& action) { Action = action; }
    unsigned int getTtl() const { return ttl; }
    void setTtl(unsigned int ttl) { this->ttl = ttl; }

    std::string str() const;

    virtual bool isLocatorExisting(const L3Address& address) const;
    virtual void addLocator(LispRlocator& entry);
    virtual LispRlocator *getLocator(const L3Address& address);
    virtual void removeLocator(L3Address& address);

    // TODO getBestUnicastLocator() (weighted RLOC selection) is added with the data plane,
    // where the owning module's RNG is available.
};

std::ostream& operator<<(std::ostream& os, const LispMapEntry& me);
std::ostream& operator<<(std::ostream& os, const Locators& rlocs);

} // namespace lisp
} // namespace inet

#endif
