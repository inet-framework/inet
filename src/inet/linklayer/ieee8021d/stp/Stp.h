//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) ANSA Team
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: ANSA Team
//

#ifndef __INET_STP_H
#define __INET_STP_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpdu_m.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dInterfaceData.h"
#include "inet/linklayer/ieee8021d/common/StpBase.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

/**
 * Implements the Spanning Tree Protocol. See the NED file for details.
 */
class INET_API Stp : public StpBase
{
  public:
    typedef Ieee8021dInterfaceData::PortInfo PortInfo;

  protected:
    static const double tickInterval; // interval between two ticks
    bool isRoot = false;
    unsigned int rootInterfaceId = 0;
    std::vector<unsigned int> desPorts; // set of designated ports

    // Discovered values
    unsigned int rootPathCost = 0;
    unsigned int rootPriority = 0;
    MacAddress rootAddress;

    uint16_t configuredBridgePriority = 0;
    simtime_t configuredMaxAge;
    simtime_t configuredForwardDelay;
    simtime_t configuredHelloInterval;

    uint16_t bridgePriority = 0;
    simtime_t maxAge;
    simtime_t forwardDelay;
    simtime_t helloInterval;

    simtime_t timeSinceLastHello;
    simtime_t holdTime;

    // Parameter change detection
    // Topology change commencing
    bool topologyChangeNotification = false;
    bool topologyChangeRecvd = false;

    PortInfo defaultPort;
    cMessage *tick = nullptr;

  public:
    Stp();
    virtual ~Stp();

    /*
     * Bridge Protocol Data Unit handling
     */
    void handleBpdu(Packet *packet, const Ptr<const BpduCfg>& bpdu);
    virtual void initInterfacedata(unsigned int interfaceId);

    /**
     * Topology change handling
     */
    void handleTcn(Packet *packet, const Ptr<const BpduTcn>& tcn);

    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /*
     * Send BPDU with specified parameters (portNum, TCA flag, etc.)
     */
    void generateBpdu(int interfaceId, const MacAddress& address = MacAddress::STP_MULTICAST_ADDRESS, bool tcFlag = false, bool tcaFlag = false);

    /*
     * Send hello BPDUs on all ports (only for root switches)
     * Invokes generateBPDU(i) where i goes through all ports
     */
    void generateHelloBpdus();

    /*
     * Generate and send Topology Change Notification
     */
    void generateTcn();

    /*
     * Comparison of all IDs in Ieee8021dInterfaceData::PortInfo structure
     * Invokes: superiorID(), superiorPort()
     */
    int comparePorts(const Ieee8021dInterfaceData *portA, const Ieee8021dInterfaceData *portB);
    int compareBridgeIDs(unsigned int aPriority, MacAddress aAddress, unsigned int bPriority, MacAddress bAddress);
    int comparePortIDs(unsigned int aPriority, unsigned int aNum, unsigned int bPriority, unsigned int bNum);

    /*
     * Check of the received BPDU is superior to port information from InterfaceTable
     */
    bool isSuperiorBpdu(int interfaceId, const Ptr<const BpduCfg>& bpdu);
    void setSuperiorBpdu(int interfaceId, const Ptr<const BpduCfg>& bpdu);

    void handleTick();

    /*
     * Check timers to make state changes
     */
    void checkTimers();
    void checkParametersChange();

    /*
     * Set the default port information for the InterfaceTable
     */
    void initPortTable();
    void selectRootPort();
    void selectDesignatedPorts();

    /*
     * Set all ports to designated (for root switch)
     */
    void setAllDesignated();

    /*
     * Helper functions to handle state changes
     */
    void lostRoot();
    void lostNonDesignated();
    void reset();

    /*
     * Determine who is eligible to become the root switch
     */
    bool checkRootEligibility();
    void tryRoot();

  public:
    friend inline std::ostream& operator<<(std::ostream& os, const Ieee8021dInterfaceData::PortRole r);
    friend inline std::ostream& operator<<(std::ostream& os, const Ieee8021dInterfaceData::PortState s);
    friend inline std::ostream& operator<<(std::ostream& os, Ieee8021dInterfaceData *p);
    friend inline std::ostream& operator<<(std::ostream& os, const Stp& i);


  protected:
    // for lifecycle:
    virtual void start() override;
    virtual void stop() override;
};

inline std::ostream& operator<<(std::ostream& os, const Ieee8021dInterfaceData::PortRole r)
{
    switch (r) {
        case Ieee8021dInterfaceData::NOTASSIGNED:
            os << "Unkn";
            break;

        case Ieee8021dInterfaceData::ALTERNATE:
            os << "Altr";
            break;

        case Ieee8021dInterfaceData::DESIGNATED:
            os << "Desg";
            break;

        case Ieee8021dInterfaceData::ROOT:
            os << "Root";
            break;

        default:
            os << "<?>";
            break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Ieee8021dInterfaceData::PortState s)
{
    switch (s) {
        case Ieee8021dInterfaceData::DISCARDING:
            os << "DIS";
            break;

        case Ieee8021dInterfaceData::LEARNING:
            os << "LRN";
            break;

        case Ieee8021dInterfaceData::FORWARDING:
            os << "FWD";
            break;

        case Ieee8021dInterfaceData::BLOCKING:
            os << "BLK";
            break;

        case Ieee8021dInterfaceData::LISTENING:
            os << "LIS";
            break;

        default:
            os << "<?>";
            break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, Ieee8021dInterfaceData *p)
{
    os << "[";
    if (p->isLearning())
        os << "L";
    else
        os << "_";
    if (p->isForwarding())
        os << "F";
    else
        os << "_";
    os << "]";

    os << " " << p->getRole() << " " << p->getState() << " ";
    os << p->getLinkCost() << " ";
    os << p->getPriority() << " ";

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Stp& i)
{
    os << "isRoot=" << i.isRoot << "\n";
    os << "bridgeAddress=" << i.bridgeAddress << "\n";
    os << "configuredBridgePriority=" << i.configuredBridgePriority << "\n";
    os << "configuredHelloInterval=" << i.configuredHelloInterval << "\n";
    os << "configuredMaxAge=" << i.configuredMaxAge << "\n";
    os << "configuredForwardDelay=" << i.configuredForwardDelay << "\n";
    os << "rootPriority=" << i.rootPriority << " rootAddress=" << i.rootAddress << "\n";
    os << "rootPathCost=" << i.rootPathCost << " rootInterfaceId=" << i.rootInterfaceId << "\n";
    os << "bridgePriority=" << i.bridgePriority << "\n";
    os << "helloInterval=" << i.helloInterval << " maxAge=" << i.maxAge << " forwardDelay=" << i.forwardDelay << "\n";
    os << "timeSinceLastHello=" << i.timeSinceLastHello << "\n";
    os << "Port  Role  State  Cost  Priority\n";
    os << "----------------------------------\n";
    for (unsigned int x = 0; x < i.numPorts; x++)
        os << x << "  " << i.getPortInterfaceData(x) << "\n";
    return os;
}

} // namespace inet

#endif

