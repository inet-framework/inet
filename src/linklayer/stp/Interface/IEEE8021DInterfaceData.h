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

#ifndef IEEE8021DINTERFACEDATA_H_
#define IEEE8021DINTERFACEDATA_H_

#include "INETDefs.h"
#include "InterfaceEntry.h"
#include "MACAddress.h"

class IEEE8021DInterfaceData : public InterfaceProtocolData
{
    public:

        //IEEE8021DInterfaceData() {}; // todo: set the port cost according to the channel bandwidth

        enum PortRole
        {
            ALTERNATE,
            NOTASSIGNED,
            DISABLED,
            DESIGNATED,
            BACKUP,
            ROOT
        };

        enum PortState
        {
            DISCARDING,
            LEARNING,
            FORWARDING
        };

        struct PortInfo
        {
            PortState state;
            PortRole role;

            unsigned int priority;
            unsigned int rootPathCost;
            unsigned int rootPriority;
            unsigned int bridgePriority;
            unsigned int portPriority;
            MACAddress rootAddress;
            MACAddress bridgeAddress;
            unsigned int portNum;
            unsigned int linkCost;
            unsigned int age;
            unsigned int fdWhile;

            unsigned int maxAge;
            unsigned int fwdDelay;
            unsigned int helloTime;
        };

    protected:
        PortInfo portData;

    public:
        unsigned int getAge() const;
        void setAge(unsigned int age);
        const MACAddress& getBridgeAddress() const;
        void setBridgeAddress(const MACAddress& bridgeAddress);
        unsigned int getBridgePriority() const;
        void setBridgePriority(unsigned int bridgePriority);
        unsigned int getFdWhile() const;
        void setFdWhile(unsigned int fdWhile);
        unsigned int getFwdDelay() const;
        void setFwdDelay(unsigned int fwdDelay);
        unsigned int getHelloTime() const;
        void setHelloTime(unsigned int helloTime);
        unsigned int getLinkCost() const;
        void setLinkCost(unsigned int linkCost);
        unsigned int getMaxAge() const;
        void setMaxAge(unsigned int maxAge);
        unsigned int getPortNum() const;
        void setPortNum(unsigned int portNum);
        unsigned int getPortPriority() const;
        void setPortPriority(unsigned int portPriority);
        unsigned int getPriority() const;
        void setPriority(unsigned int priority);
        PortRole getRole() const;
        void setRole(PortRole role);
        const MACAddress& getRootAddress() const;
        void setRootAddress(const MACAddress& rootAddress);
        unsigned int getRootPathCost() const;
        void setRootPathCost(unsigned int rootPathCost);
        unsigned int getRootPriority() const;
        void setRootPriority(unsigned int rootPriority);
        PortState getState() const;
        void setState(PortState state);

        PortInfo getPortInfoData();
        void setPortInfoData(PortInfo& portInfoData);
        bool isLearning();
        bool isForwarding();
};

#endif /* IEEE8021DINTERFACEDATA_H_ */
