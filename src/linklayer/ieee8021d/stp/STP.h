//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) ANSA Team
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
// Authors: ANSA Team, Benjamin Martin Seregi
//

#ifndef INET_SPANNINGTREE_H_
#define INET_SPANNINGTREE_H_

#include "INETDefs.h"
#include "MACAddress.h"
#include "MACAddressTable.h"
#include "IEEE8021DBPDU_m.h"
#include "InterfaceTable.h"
#include "Ieee8021DInterfaceData.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "STPBase.h"

/**
 * Implements the Spanning Tree Protocol. See the NED file for details.
 */
class STP : public STPBase
{
    public:
        typedef Ieee8021DInterfaceData::PortInfo PortInfo;

    protected:

        int convergenceTime;
        bool isRoot;
        unsigned int rootPort;
        std::vector<unsigned int> desPorts; // set of designated ports

        // Discovered values
        unsigned int rootPathCost;
        unsigned int rootPriority;
        MACAddress rootAddress;

        // Set by root bridge, c stands for current
        simtime_t cMaxAge;
        simtime_t cFwdDelay;
        simtime_t cHelloTime;

        // Parameter change detection
        unsigned int ubridgePriority;

        simtime_t helloTimer;

        // Topology change commencing
        bool topologyChangeNotification;
        bool topologyChangeRecvd;

        PortInfo defaultPort;
        cMessage * tick;

    public:

        /*
         * Bridge Protocol Data Unit handling
         */
        void handleBPDU(BPDU * bpdu);

        /**
         * Topology change handling
         */
        void handleTCN(BPDU * tcn);
        virtual void handleMessage(cMessage * msg);
        virtual void initialize(int stage);
        virtual int numInitStages() const { return 2; }
        virtual ~STP();

        /*
         * Generate BPDUs to all interfaces (for root switch)
         */
        void generateBPDU(int portNum, const MACAddress& address = MACAddress::STP_MULTICAST_ADDRESS, bool tcFlag = false, bool tcaFlag = false);
        void generator();

        /*
         * Generate and send Topology Change Notification
         */
        void generateTCN();

        /*
         * Comparison of all IDs in IEEE8021DInterfaceData::PortInfo structure
         * Invokes: superiorID(), superiorPort()
         */
        int superiorTPort(Ieee8021DInterfaceData * portA, Ieee8021DInterfaceData * portB);
        int superiorID(unsigned int, MACAddress, unsigned int, MACAddress);
        int superiorPort(unsigned int, unsigned int, unsigned int, unsigned int);

        /*
         * Check of the received BPDU is superior to port information from InterfaceTable
         */
        bool superiorBPDU(int portNum, BPDU * bpdu);
        void setSuperiorBPDU(int portNum, BPDU * bpdu);

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
        void allDesignated();

        /*
         * State changes
         */
        void lostRoot();
        void lostAlternate(int port);
        void reset();

        /*
         * Determine who is eligible to become the root switch
         */
        bool checkRootEligibility();
        void tryRoot();

    public:
        friend inline std::ostream& operator<<(std::ostream& os, const Ieee8021DInterfaceData::PortRole r);
        friend inline std::ostream& operator<<(std::ostream& os, const Ieee8021DInterfaceData::PortState s);
        friend inline std::ostream& operator<<(std::ostream& os, Ieee8021DInterfaceData * p);
        friend inline std::ostream& operator<<(std::ostream& os, STP i);

        // for lifecycle:
    protected:
        virtual void start();
        virtual void stop();
};

inline std::ostream& operator<<(std::ostream& os, const Ieee8021DInterfaceData::PortRole r)
{

    switch (r)
    {
        case Ieee8021DInterfaceData::NOTASSIGNED:
            os << "Unkn";
            break;
        case Ieee8021DInterfaceData::ALTERNATE:
            os << "Altr";
            break;
        case Ieee8021DInterfaceData::DESIGNATED:
            os << "Desg";
            break;
        case Ieee8021DInterfaceData::ROOT:
            os << "Root";
            break;
        default:
            os << "<?>";
            break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Ieee8021DInterfaceData::PortState s)
{

    switch (s)
    {
        case Ieee8021DInterfaceData::DISCARDING:
            os << "DIS";
            break;
        case Ieee8021DInterfaceData::LEARNING:
            os << "LRN";
            break;
        case Ieee8021DInterfaceData::FORWARDING:
            os << "FWD";
            break;
        default:
            os << "<?>";
            break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, Ieee8021DInterfaceData * p)
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

inline std::ostream& operator<<(std::ostream& os, STP i)
{
    os << "RootID Priority: " << i.rootPriority << " \n";
    os << "  Address: " << i.rootAddress << " \n";
    if (i.isRoot)
        os << "  This bridge is the Root. \n";
    else
    {
        os << "  Cost: " << i.rootPathCost << " \n";
        os << "  Port: " << i.rootPort << " \n";
    }
    os << "  Hello Time: " << i.cHelloTime << " \n";
    os << "  Max Age: " << i.cMaxAge << " \n";
    os << "  Forward Delay: " << i.cFwdDelay << " \n";
    os << "BridgeID Priority: " << i.bridgePriority << "\n";
    os << "  Address: " << i.bridgeAddress << " \n";
    os << "  Hello Time: " << i.helloTime << " \n";
    os << "  Max Age: " << i.maxAge << " \n";
    os << "  Forward Delay: " << i.forwardDelay << " \n";
    os << "Port Flag Role State Cost Priority \n";
    os << "-----------------------------------------\n";

    for (unsigned int x = 0; x < i.numPorts; x++)
        os << x << "  " << i.getPortInterfaceData(x) << " \n";

    return os;
}

#endif
