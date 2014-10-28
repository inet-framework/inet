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

#ifndef INET_STPTESTER_H_
#define INET_STPTESTER_H_

#include <map>
#include "INETDefs.h"
#include "Topology.h"
#include "STP.h"

/**
 * Utility class for testing the STP protocol. First, it extracts the network
 * topology (network nodes marked with the @node NED property), regarding the
 * enabled (state=FORWARDING) links only. Then it analyzes the resulting graph
 * for connectedness and loop free-ness, using a modified depth-first search
 * with cycle detection. The results can be obtained with getter methods.
 */
// TODO: this module should be moved into the test folder somewhere
class INET_API STPTester : public cSimpleModule
{
    public:
        enum Color
        {
            WHITE, GRAY, BLACK
        };

    protected:
        bool loop;
        int numOfVisitedNodes;
        int numOfNodes;
        std::map<Topology::Node *, int> color;
        std::map<Topology::Node *, Topology::Node *> parent;
        Topology graph;

        simtime_t checkTime;
        cMessage* checkTimer;

    public:
        // Includes network topology extraction
        STPTester();
        ~STPTester();
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

    protected:
        void dfsVisit(Topology::Node * node);
        bool isForwarding(Topology::Node * node, unsigned int portNum);

        // Analyzes the network graph
        void depthFirstSearch();

        // Getters for returning the result after a call to depthFirstSearch()
        bool isLoopFreeGraph();
        bool isConnectedGraph();
        bool isTreeGraph();

        int getNumOfNodes();
        int getNumOfVisitedNodes();
};

#endif
