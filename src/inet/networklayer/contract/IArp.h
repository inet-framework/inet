//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IARP_H
#define __INET_IARP_H

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class NetworkInterface;

/**
 * Represents an Ipv4 ARP module.
 */
class INET_API IArp
{
  public:
    /**
     * Sent in ARP cache change notification signals
     */
    class INET_API Notification : public cObject {
      public:
        L3Address l3Address;
        MacAddress macAddress;
        const NetworkInterface *ie;

      public:
        Notification(L3Address l3Address, MacAddress macAddress, const NetworkInterface *ie)
            : l3Address(l3Address), macAddress(macAddress), ie(ie) {}
    };

    /** @brief Signals used to publish ARP state changes. */
    static const simsignal_t arpResolutionInitiatedSignal;
    static const simsignal_t arpResolutionCompletedSignal;
    static const simsignal_t arpResolutionFailedSignal;
    /**
     * Emitted when a reply names an address this host is probing, which means the address is
     * in use by somebody else. RFC 5227 section 2.1.1. The notification carries the probed
     * address and the hardware address that answered for it.
     */
    static const simsignal_t arpAddressConflictDetectedSignal;

  public:
    virtual ~IArp() {}

    /**
     * Returns the Layer 3 address for the given MAC address. If it is not available
     * (not in the cache, pending resolution, or already expired), UNSPECIFIED_ADDRESS
     * is returned.
     */
    virtual L3Address getL3AddressFor(const MacAddress&) const = 0;

    /**
     * Tries to resolve the given network address to a MAC address. If the MAC
     * address is not yet resolved it returns an unspecified address and starts
     * an address resolution procedure. A signal is emitted when the address
     * resolution procedure terminates.
     */
    virtual MacAddress resolveL3Address(const L3Address& address, const NetworkInterface *ie) = 0;

    /**
     * Asks whether anybody already holds the given address, by sending an ARP Probe: a
     * request that carries an all-zero sender protocol address, so it claims nothing. RFC
     * 5227 section 2.1.1. An answer, if one comes, is reported through
     * arpAddressConflictDetectedSignal.
     *
     * An implementation that resolves addresses without packets cannot ask the question, so
     * it sends nothing and reports no conflict. A caller must therefore treat silence as
     * "nobody holds it", which is what the standard asks of it anyway.
     */
    virtual void sendArpProbe(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address probedAddr) = 0;
};

} // namespace inet

#endif

