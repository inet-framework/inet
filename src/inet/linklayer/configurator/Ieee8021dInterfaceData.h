//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021DINTERFACEDATA_H
#define __INET_IEEE8021DINTERFACEDATA_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

#undef ALTERNATE // conflicts with <windows.h>

/**
 * Per-interface data needed by the STP and RSTP protocols.
 */
class INET_API Ieee8021dInterfaceData : public InterfaceProtocolData
{
  public:
    enum PortRole { ALTERNATE, NOTASSIGNED, DISABLED, DESIGNATED, BACKUP, ROOT };

    enum PortState { DISCARDING, LEARNING, FORWARDING };

    class INET_API PortInfo {
      public:
        /* The following values have same meaning in both STP and RSTP.
         * See Ieee8021dBDPU for more info.
         */
        unsigned int priority;
        unsigned int linkCost;
        bool edge;

        PortState state;
        PortRole role;

        unsigned int rootPriority;
        MacAddress rootAddress;
        unsigned int rootPathCost;
        unsigned int bridgePriority;
        MacAddress bridgeAddress;
        unsigned int portPriority;
        unsigned int portNum; // The number of the switch port (i.e. EthernetSwitch ethg[] gate index).

        simtime_t age; // This parameter is conveyed to enable a switch to discard information whose age exceeds Max Age. (STP,RSTP)
        simtime_t maxAge;

        simtime_t fdWhile; // Forward delay timer (see fwdDelay). (STP,RSTP)
        simtime_t fwdDelay; // The time spent by a Port in the Listening State and the Learning State before moving to the Learning or For-
                            // warding State, respectively. (STP,RSTP)

        simtime_t helloTime; // The time interval between the generation of Configuration BPDUs by the Root. (STP)
                             // The interval between periodic transmissions of Configuration Messages by Designated Ports. (RSTP)

        simtime_t TCWhile; // The Topology Change timer. TCN Messages are sent while this timer is running. (RSTP)

        unsigned int lostBPDU;
        simtime_t nextUpgrade;

      public:
        PortInfo();
    };

  protected:
    PortInfo portData;

  public:
    Ieee8021dInterfaceData();

    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    bool isLearning() const { return portData.state == LEARNING || portData.state == FORWARDING; }

    bool isForwarding() const { return portData.state == FORWARDING; }

    simtime_t getAge() const { return portData.age; }

    void setAge(simtime_t age) { portData.age = age; }

    const MacAddress& getBridgeAddress() const { return portData.bridgeAddress; }

    void setBridgeAddress(const MacAddress& bridgeAddress) { portData.bridgeAddress = bridgeAddress; }

    unsigned int getBridgePriority() const { return portData.bridgePriority; }

    void setBridgePriority(unsigned int bridgePriority) { portData.bridgePriority = bridgePriority; }

    simtime_t getFdWhile() const { return portData.fdWhile; }

    void setFdWhile(simtime_t fdWhile) { portData.fdWhile = fdWhile; }

    simtime_t getFwdDelay() const { return portData.fwdDelay; }

    void setFwdDelay(simtime_t fwdDelay) { portData.fwdDelay = fwdDelay; }

    simtime_t getHelloTime() const { return portData.helloTime; }

    void setHelloTime(simtime_t helloTime) { portData.helloTime = helloTime; }

    unsigned int getLinkCost() const { return portData.linkCost; }

    void setLinkCost(unsigned int linkCost) { portData.linkCost = linkCost; }

    simtime_t getMaxAge() const { return portData.maxAge; }

    void setMaxAge(simtime_t maxAge) { portData.maxAge = maxAge; }

    unsigned int getPortNum() const { return portData.portNum; }

    void setPortNum(unsigned int portNum) { portData.portNum = portNum; }

    unsigned int getPortPriority() const { return portData.portPriority; }

    void setPortPriority(unsigned int portPriority) { portData.portPriority = portPriority; }

    unsigned int getPriority() const { return portData.priority; }

    void setPriority(unsigned int priority) { portData.priority = priority; }

    PortRole getRole() const { return portData.role; }

    void setRole(PortRole role) { portData.role = role; }

    const MacAddress& getRootAddress() const { return portData.rootAddress; }

    void setRootAddress(const MacAddress& rootAddress) { portData.rootAddress = rootAddress; }

    unsigned int getRootPathCost() const { return portData.rootPathCost; }

    void setRootPathCost(unsigned int rootPathCost) { portData.rootPathCost = rootPathCost; }

    unsigned int getRootPriority() const { return portData.rootPriority; }

    void setRootPriority(unsigned int rootPriority) { portData.rootPriority = rootPriority; }

    PortState getState() const { return portData.state; }

    void setState(PortState state) { portData.state = state; }

    bool isEdge() const { return portData.edge; }

    void setEdge(bool edge) { portData.edge = edge; }

    simtime_t getTCWhile() const { return portData.TCWhile; }

    void setTCWhile(simtime_t TCWhile) { portData.TCWhile = TCWhile; }

    unsigned int getLostBPDU() const { return portData.lostBPDU; }

    void setLostBPDU(unsigned int lostBPDU) { portData.lostBPDU = lostBPDU; }

    const char *getRoleName() const { return getRoleName(getRole()); }

    const char *getStateName() const { return getStateName(getState()); }

    static const char *getRoleName(PortRole role);

    static const char *getStateName(PortState state);

    simtime_t getNextUpgrade() const { return portData.nextUpgrade; }

    void setNextUpgrade(simtime_t nextUpgrade) { portData.nextUpgrade = nextUpgrade; }
};

} // namespace inet

#endif

