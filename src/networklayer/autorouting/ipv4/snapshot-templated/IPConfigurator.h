//
// Copyright (C) 2011 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPCONFIGURATOR_H
#define __INET_IPCONFIGURATOR_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "Topology.h"
#include "IInterfaceTable.h"
#include "IRoutingTable.h"

// compile time macros to disable or enable logging
#define EV_ENABLED EV
#define EV_DISABLED true?ev:ev

// compile time log levels
#define EV_DEBUG EV_ENABLED
#define EV_INFO EV_ENABLED

/**
 * This class serves as a base class for IP addresses configurators. The class
 * takes a integer typename as template parameter to efficiently represent IP addresses.
 * The provided type must support the usual integer operations (+ & | >> etc.)
 */
template <typename IPUINT>
class INET_API IPConfigurator : public cSimpleModule
{
    public:
        class NodeInfo;

        class LinkInfo;

        /**
         * Stores information attached to an interface in the network.
         */
        class InterfaceInfo : public cObject {
            public:
                NodeInfo *nodeInfo;
                LinkInfo *linkInfo;
                InterfaceEntry *interfaceEntry;
                bool configure;              // false means the IP address of the interface will not be modified
                IPUINT address;              // the bits
                IPUINT addressSpecifiedBits; // 1 means the bit is specifiedc, 0 means the bit is unspecified
                IPUINT netmask;              // the bits
                IPUINT netmaskSpecifiedBits; // 1 means the bit is specifiedc, 0 means the bit is unspecified

                InterfaceInfo(NodeInfo *nodeInfo, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry) {
                    this->nodeInfo = nodeInfo;
                    this->linkInfo = linkInfo;
                    this->interfaceEntry = interfaceEntry;
                    configure = true;
                    address = 0;
                    addressSpecifiedBits = 0;
                    netmask = 0;
                    netmaskSpecifiedBits = 0;
                }
                virtual std::string getFullPath() const { return interfaceEntry->getFullPath(); }
        };

        /**
         * Stores information attached to a node in the network.
         * Used as the payload of nodes in the topology.
         */
        class NodeInfo : public cObject {
            public:
                bool isIPNode; // true means the node has an interface table
                cModule *module;
                IInterfaceTable *interfaceTable;
                IRoutingTable *routingTable;
                std::vector<InterfaceInfo*> interfaceInfos;

                NodeInfo(cModule *module) { this->module = module; isIPNode = false; interfaceTable = NULL; routingTable = NULL; }
                virtual std::string getFullPath() const { return module->getFullPath(); }
        };

        /**
         * Stores information attached to a link in the network.
         * A link is a shared communication medium between nodes (e.g. ethernet, wireless).
         */
        class LinkInfo : public cObject {
            public:
                std::vector<InterfaceInfo*> interfaceInfos; // interfaces on that LAN or point-to-point link
                InterfaceInfo* gatewayInterfaceInfo; // non NULL if all hosts have 1 non-loopback interface except one host that has two of them (this will be the gateway)

                LinkInfo() { gatewayInterfaceInfo = NULL; }
                ~LinkInfo() { for (int i = 0; i < interfaceInfos.size(); i++) delete interfaceInfos[i]; }
        };

        class NetworkInfo : public cObject {  //TODO put Topology* into it
            public:
                std::vector<LinkInfo*> linkInfos; // all links in the network

                ~NetworkInfo() { for (int i = 0; i < linkInfos.size(); i++) delete linkInfos[i]; }
        };

    protected:
        virtual int numInitStages() const  { return 3; }
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }

        /**
         * Extracts network topology by walking through the module hierarchy.
         * Creates vertices from modules having @node property.
         * Creates edges from connections between network interfaces.
         */
        virtual void extractTopology(Topology& topology, NetworkInfo& networkInfo);

        /**
         * Assigns IP addresses to InterfaceEntries based on InterfaceInfo parameters.
         * See the NED file for details.
         */
        virtual void assignAddresses(Topology& topology, NetworkInfo& networkInfo);
        virtual void dumpAddresses(NetworkInfo& networkInfo);

        // helper functions
        virtual void extractNeighbors(Topology::LinkOut *linkOut, LinkInfo* linkInfo, std::set<InterfaceEntry*>& interfacesSeen, std::vector<Topology::Node*>& nodesVisited);
        virtual InterfaceInfo *createInterfaceInfo(NodeInfo *nodeInfo, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);

        /**
         * Called by assignAddresses to actually assign an address to an interface.
         */
        virtual void assignAddress(InterfaceEntry *interfaceEntry, IPUINT address, IPUINT netmask) = 0;
        virtual std::string toString(IPUINT value) = 0;
};

#endif
