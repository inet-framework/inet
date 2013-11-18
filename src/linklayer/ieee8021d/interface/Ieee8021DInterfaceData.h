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

#ifndef INET_IEEE8021DINTERFACEDATA_H_
#define INET_IEEE8021DINTERFACEDATA_H_

#include "INETDefs.h"
#include "InterfaceEntry.h"
#include "MACAddress.h"

#undef ALTERNATE  // conflicts with <windows.h>


/**
 * Per-interface data needed by the STP and RSTP protocols.
 */
class Ieee8021DInterfaceData : public InterfaceProtocolData
{
    public:

        Ieee8021DInterfaceData();
        enum PortRole
        {
            ALTERNATE, NOTASSIGNED, DISABLED, DESIGNATED, BACKUP, ROOT
        };

        enum PortState
        {
            DISCARDING, LEARNING, FORWARDING
        };

        struct PortInfo
        {
                /* The following values have same meaning in both STP and RSTP.
                 * See IEEE8021DBDPU for more info.
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
                unsigned int portNum;       // The number of the switch port (i.e. EtherSwitch ethg[] gate index).


                simtime_t age;              // This parameter is conveyed to enable a switch to discard information whose age exceeds Max Age. (STP,RSTP)
                simtime_t maxAge;

                simtime_t fdWhile;          // Forward delay timer (see fwdDelay). (STP,RSTP)
                simtime_t fwdDelay;         // The time spent by a Port in the Listening State and the Learning State before moving to the Learning or For-
                                            // warding State, respectively. (STP,RSTP)

                simtime_t helloTime;        // The time interval between the generation of Configuration BPDUs by the Root. (STP)
                                            // The interval between periodic transmissions of Configuration Messages by Designated Ports. (RSTP)

                simtime_t TCWhile;          // The Topology Change timer. TCN Messages are sent while this timer is running. (RSTP)

                unsigned int lostBPDU;
        };

    protected:
        PortInfo portData;
        PortInfo defaultStpPort;

    public:
        inline bool isLearning()
        {
            return portData.state == LEARNING || portData.state == FORWARDING;
        }

        inline bool isForwarding()
        {
            return portData.state == FORWARDING;
        }

        inline simtime_t getAge() const
        {
            return portData.age;
        }

        inline void setAge(simtime_t age)
        {
            portData.age = age;
        }

        inline const MACAddress& getBridgeAddress() const
        {
            return portData.bridgeAddress;
        }

        inline void setBridgeAddress(const MACAddress& bridgeAddress)
        {
            portData.bridgeAddress = bridgeAddress;
        }

        inline unsigned int getBridgePriority() const
        {
            return portData.bridgePriority;
        }

        inline void setBridgePriority(unsigned int bridgePriority)
        {
            portData.bridgePriority = bridgePriority;
        }

        inline simtime_t getFdWhile() const
        {
            return portData.fdWhile;
        }

        inline void setFdWhile(simtime_t fdWhile)
        {
            portData.fdWhile = fdWhile;
        }

        inline simtime_t getFwdDelay() const
        {
            return portData.fwdDelay;
        }

        inline void setFwdDelay(simtime_t fwdDelay)
        {
            portData.fwdDelay = fwdDelay;
        }

        inline simtime_t getHelloTime() const
        {
            return portData.helloTime;
        }

        inline void setHelloTime(simtime_t helloTime)
        {
            portData.helloTime = helloTime;
        }

        inline unsigned int getLinkCost() const
        {
            return portData.linkCost;
        }

        inline void setLinkCost(unsigned int linkCost)
        {
            portData.linkCost = linkCost;
        }

        inline simtime_t getMaxAge() const
        {
            return portData.maxAge;
        }

        inline void setMaxAge(simtime_t maxAge)
        {
            portData.maxAge = maxAge;
        }

        inline unsigned int getPortNum() const
        {
            return portData.portNum;
        }

        inline void setPortNum(unsigned int portNum)
        {
            portData.portNum = portNum;
        }

        inline unsigned int getPortPriority() const
        {
            return portData.portPriority;
        }

        inline void setPortPriority(unsigned int portPriority)
        {
            portData.portPriority = portPriority;
        }

        inline unsigned int getPriority() const
        {
            return portData.priority;
        }

        inline void setPriority(unsigned int priority)
        {
            portData.priority = priority;
        }

        inline PortRole getRole() const
        {
            return portData.role;
        }

        inline void setRole(PortRole role)
        {
            portData.role = role;
        }

        inline const MACAddress& getRootAddress() const
        {
            return portData.rootAddress;
        }

        inline void setRootAddress(const MACAddress& rootAddress)
        {
            portData.rootAddress = rootAddress;
        }

        inline unsigned int getRootPathCost() const
        {
            return portData.rootPathCost;
        }

        inline void setRootPathCost(unsigned int rootPathCost)
        {
            portData.rootPathCost = rootPathCost;
        }

        inline unsigned int getRootPriority() const
        {
            return portData.rootPriority;
        }

        inline void setRootPriority(unsigned int rootPriority)
        {
            portData.rootPriority = rootPriority;
        }

        inline PortState getState() const
        {
            return portData.state;
        }

        inline void setState(PortState state)
        {
            portData.state = state;
        }

        inline PortInfo getPortInfoData()
        {
            return portData;
        }

        inline void setDefaultStpPortInfoData()
        {
            portData = defaultStpPort;
        }

        inline bool isEdge() const
        {
            return portData.edge;
        }

        inline void setEdge(bool edge)
        {
            portData.edge=edge;
        }

        inline simtime_t getTCWhile() const
        {
            return portData.TCWhile;
        }

        inline void setTCWhile(simtime_t TCWhile)
        {
            portData.TCWhile = TCWhile;
        }

        inline unsigned int getLostBPDU() const
        {
            return portData.lostBPDU;
        }

        inline void setLostBPDU(unsigned int lostBPDU)
        {
            portData.lostBPDU = lostBPDU;
        }

};

#endif
