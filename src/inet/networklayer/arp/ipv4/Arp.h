//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ARP_H
#define __INET_ARP_H

#include <map>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

// Forward declarations:
class ArpPacket;
class IInterfaceTable;
class NetworkInterface;
class IIpv4RoutingTable;

/**
 * ARP implementation.
 */
class INET_API Arp : public OperationalBase, public IArp
{
  public:
    class ArpCacheEntry;
    typedef std::map<Ipv4Address, ArpCacheEntry *> ArpCache;
    typedef std::vector<cMessage *> MsgPtrVector;

    // Ipv4Address -> MacAddress table
    // TODO should we key it on (Ipv4Address, NetworkInterface*)?
    class INET_API ArpCacheEntry {
      public:
        Arp *owner = nullptr; // owner ARP module of this cache entry
        const NetworkInterface *ie = nullptr; // NIC to send the packet to
        bool pending = false; // true if resolution is pending
        MacAddress macAddress; // MAC address
        simtime_t lastUpdate; // entries should time out after cacheTimeout
        int numRetries = 0; // if pending==true: 0 after first ARP request, 1 after second, etc.
        cMessage *timer = nullptr; // if pending==true: request timeout msg
        Ipv4Address ipv4Address;
    };

  protected:
    simtime_t retryTimeout;
    int retryCount = 0;
    simtime_t cacheTimeout;
    std::string proxyArpInterfaces = "";
    long numResolutions = 0;
    long numFailedResolutions = 0;
    long numRequestsSent = 0;
    long numRepliesSent = 0;

    cPatternMatcher proxyArpInterfacesMatcher;

    static simsignal_t arpRequestSentSignal;
    static simsignal_t arpReplySentSignal;

    ArpCache arpCache;

    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt; // for answering ProxyARP requests

  protected:
    // Maps an IP multicast address to an Ethernet multicast address.
    MacAddress mapMulticastAddress(Ipv4Address addr);

  public:
    Arp();
    virtual ~Arp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /// IArp implementation  @{
    virtual MacAddress resolveL3Address(const L3Address& address, const NetworkInterface *ie) override;
    virtual L3Address getL3AddressFor(const MacAddress& addr) const override;
    ///@}

    void sendArpGratuitous(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address ipAddr, ArpOpcode opCode = ARP_REQUEST);
    void sendArpProbe(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address probedAddr);

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    // Lifecycle methods
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void flush();

    virtual void initiateArpResolution(Ipv4Address ipv4Address, ArpCacheEntry *entry);
    virtual void sendArpRequest(const NetworkInterface *ie, Ipv4Address ipAddress);
    virtual void requestTimedOut(cMessage *selfmsg);
    virtual bool addressRecognized(Ipv4Address destAddr, NetworkInterface *ie);
    virtual void processArpPacket(Packet *packet);
    virtual void updateArpCache(ArpCacheEntry *entry, const MacAddress& macAddress);

    virtual MacAddress resolveMacAddressForArpReply(const NetworkInterface *ie, const ArpPacket *arp);

    virtual void dumpArpPacket(const ArpPacket *arp);
    virtual void refreshDisplay() const override;
};

} // namespace inet

#endif

