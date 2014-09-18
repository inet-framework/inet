//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Benjamin Martin Seregi
//

#ifndef __INET_IEEE8021DINTERFACEDATA_H
#define __INET_IEEE8021DINTERFACEDATA_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

#undef ALTERNATE    // conflicts with <windows.h>

/**
 * Per-interface data needed by the STP and RSTP protocols.
 */
class Ieee8021dInterfaceData : public InterfaceProtocolData
{
  public:
    enum PortRole { ALTERNATE, NOTASSIGNED, DISABLED, DESIGNATED, BACKUP, ROOT };

    enum PortState { DISCARDING, LEARNING, FORWARDING };

    class PortInfo
    {
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
        MACAddress rootAddress;
        unsigned int rootPathCost;
        unsigned int bridgePriority;
        MACAddress bridgeAddress;
        unsigned int portPriority;
        unsigned int portNum;    // The number of the switch port (i.e. EtherSwitch ethg[] gate index).

        simtime_t age;    // This parameter is conveyed to enable a switch to discard information whose age exceeds Max Age. (STP,RSTP)
        simtime_t maxAge;

        simtime_t fdWhile;    // Forward delay timer (see fwdDelay). (STP,RSTP)
        simtime_t fwdDelay;    // The time spent by a Port in the Listening State and the Learning State before moving to the Learning or For-
                               // warding State, respectively. (STP,RSTP)

        simtime_t helloTime;    // The time interval between the generation of Configuration BPDUs by the Root. (STP)
                                // The interval between periodic transmissions of Configuration Messages by Designated Ports. (RSTP)

        simtime_t TCWhile;    // The Topology Change timer. TCN Messages are sent while this timer is running. (RSTP)

        unsigned int lostBPDU;
        simtime_t nextUpgrade;

      public:
        PortInfo();
    };

  protected:
    PortInfo portData;

  public:
    Ieee8021dInterfaceData();

    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    bool isLearning() { return portData.state == LEARNING || portData.state == FORWARDING; }

    bool isForwarding() { return portData.state == FORWARDING; }

    simtime_t getAge() const { return portData.age; }

    void setAge(simtime_t age) { portData.age = age; }

    const MACAddress& getBridgeAddress() const { return portData.bridgeAddress; }

    void setBridgeAddress(const MACAddress& bridgeAddress) { portData.bridgeAddress = bridgeAddress; }

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

    const MACAddress& getRootAddress() const { return portData.rootAddress; }

    void setRootAddress(const MACAddress& rootAddress) { portData.rootAddress = rootAddress; }

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

#endif // ifndef __INET_IEEE8021DINTERFACEDATA_H

