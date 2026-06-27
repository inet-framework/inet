//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPSERVERENTRY_H
#define __INET_LISPSERVERENTRY_H

#include <list>

#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/lisp/LispCommon.h"

namespace inet {
namespace lisp {

/**
 * A Map-Server or Map-Resolver entry: its address, the authentication key and
 * the proxy-reply / want-map-notify / quick-registration flags.
 */
class INET_API LispServerEntry
{
  private:
    L3Address address;
    std::string key;
    bool proxyReply = false;
    bool mapNotify = false;
    bool quickRegistration = false;
    simtime_t lastTime = SIMTIME_ZERO;

  public:
    LispServerEntry() {}
    explicit LispServerEntry(std::string nipv);
    LispServerEntry(std::string nipv, std::string nkey, bool proxy, bool notify, bool quick);

    bool operator==(const LispServerEntry& other) const;
    bool operator<(const LispServerEntry& other) const;

    std::string str() const;

    const std::string& getKey() const { return key; }
    void setKey(const std::string& key) { this->key = key; }
    bool isMapNotify() const { return mapNotify; }
    void setMapNotify(bool mapNotify) { this->mapNotify = mapNotify; }
    bool isProxyReply() const { return proxyReply; }
    void setProxyReply(bool proxyReply) { this->proxyReply = proxyReply; }
    bool isQuickRegistration() const { return quickRegistration; }
    void setQuickRegistration(bool quickRegistration) { this->quickRegistration = quickRegistration; }
    const L3Address& getAddress() const { return address; }
    void setAddress(const L3Address& address) { this->address = address; }
    simtime_t getLastTime() const { return lastTime; }
    void setLastTime(simtime_t lastTime) { this->lastTime = lastTime; }
};

typedef std::list<LispServerEntry> ServerAddresses;
typedef ServerAddresses::iterator ServerItem;
typedef ServerAddresses::const_iterator ServerCItem;

std::ostream& operator<<(std::ostream& os, const LispServerEntry& entry);

} // namespace lisp
} // namespace inet

#endif
