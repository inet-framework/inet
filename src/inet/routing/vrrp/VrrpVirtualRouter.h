//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_VRRPVIRTUALROUTER_H
#define __INET_VRRPVIRTUALROUTER_H

#include <vector>

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/common/VirtualForwarder.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/routing/vrrp/VrrpAdvertisement_m.h"

namespace inet {
namespace vrrp {

class Vrrp;

/**
 * A single VRRPv2 virtual router (one VRID on one interface), implementing the
 * RFC 3768 state machine. Created and driven by the parent Vrrp module: the
 * parent calls processReceivedAdvertisement() for inbound advertisements, and
 * this module calls Vrrp::sendAdvertisement() to transmit (it owns no gates).
 * It keeps its own self-message timers.
 */
class INET_API VrrpVirtualRouter : public SimpleModule
{
  public:
    enum VrrpState {
        SHUTDOWN = 0,
        INITIALIZE,
        BACKUP,
        MASTER,
    };

    enum VrrpPhase {
        INIT,
        TIMER_START,
        TIMER_END,
        LEAVE,
        STOP,
    };

    enum VrrpTimerKind {
        ADVERTISE,
        MASTERDOWN,
        BROADCAST,
        PREEMPTION,
        INITCHECK,
    };

  protected:
    static simsignal_t vrStateSignal;
    static simsignal_t sentAdvertisementSignal;
    static simsignal_t recvAdvertisementSignal;
    static simsignal_t sentArpSignal;

    Vrrp *vrrp = nullptr; // parent module (set right after creation)
    IInterfaceTable *ift = nullptr;
    NetworkInterface *ie = nullptr;
    VirtualForwarder *vforwarder = nullptr; // owned; registered on the interface

    VrrpState state = INITIALIZE;

    int vrid = 0;
    int interfaceId = -1;
    int version = 2;
    int priority = 100;
    int priorityOwner = 255;
    int ttl = 255;
    int arpType = 2;
    bool own = false;
    bool preemption = true;
    bool preemptionDelay = false;
    bool learn = false;
    std::string description;
    double arpDelay = 0;

    Ipv4Address primaryIp;                 // group (primary) virtual IP address
    std::vector<Ipv4Address> virtualIps;   // all virtual IPs (primaryIp is virtualIps[0])
    Ipv4Address multicastIp;
    MacAddress virtualMac;
    std::string virtualMacOui;

    int advertisementInterval = 1;         // configured interval, seconds (wire field)
    int advertisementIntervalActual = 1;   // currently used interval, seconds

    simtime_t masterDownTimerInit;
    simtime_t adverTimerInit;
    simtime_t preemptTimerInit;
    cMessage *masterDownTimer = nullptr;
    cMessage *adverTimer = nullptr;
    cMessage *broadcastTimer = nullptr;
    cMessage *preemptionTimer = nullptr;
    cMessage *initCheckTimer = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override; // only self-message timers
    virtual void preDelete(cComponent *root) override;

    // configuration
    void loadConfig();
    void createVirtualMac();
    void setOwn();

    // state machine
    void stateInitialize();
    void stateBackup(VrrpPhase phase);
    void stateMaster(VrrpPhase phase);

    // timers
    void handleTimer(cMessage *msg);
    void cancelTimer(cMessage *&timer);
    void scheduleAdverTimer();
    void cancelAdverTimer();
    void scheduleMasterDownTimer();
    void cancelMasterDownTimer();
    void schedulePreemptionTimer();
    void cancelPreemptionTimer();
    void scheduleBroadcastTimer();
    void cancelBroadcastTimer();
    void scheduleInitCheck();

    // advertisement handling
    void sendAdvertisement();
    void sendBroadcast();
    bool validateAdvertisement(const Ptr<const VrrpAdvertisement>& adv, int receivedTtl);
    void handleAdvertisementBackup(const Ptr<const VrrpAdvertisement>& adv, Ipv4Address srcAddr);
    void handleAdvertisementMaster(const Ptr<const VrrpAdvertisement>& adv, Ipv4Address srcAddr);

    // helpers
    simtime_t getSkewTime() const { return ((256.0 - priority) * advertisementIntervalActual) / 256.0; }
    simtime_t getMasterDownInterval() const { return 3 * advertisementIntervalActual + getSkewTime(); }
    simtime_t getAdvertisementInterval() const { return advertisementIntervalActual; }
    std::string getStateName(VrrpState state) const;
    void refreshDisplay() const override;

  public:
    VrrpVirtualRouter() {}
    virtual ~VrrpVirtualRouter();

    int getVrid() const { return vrid; }
    int getInterfaceId() const { return interfaceId; }
    int getPriority() const { return priority; }
    const Ipv4Address& getMulticastAddress() const { return multicastIp; }
    int getTimeToLive() const { return ttl; }

    /** Called by the parent Vrrp module for an inbound advertisement on this VR. */
    void processReceivedAdvertisement(const Ptr<const VrrpAdvertisement>& adv, Ipv4Address srcAddr, int receivedTtl);
};

} // namespace vrrp
} // namespace inet

#endif
