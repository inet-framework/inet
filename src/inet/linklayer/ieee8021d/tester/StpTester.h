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

#ifndef __INET_STPTESTER_H
#define __INET_STPTESTER_H

#include <map>

#include "inet/common/INETDefs.h"
#include "inet/common/Topology.h"
#include "inet/linklayer/ieee8021d/stp/Stp.h"

namespace inet {

/**
 * Utility class for testing the STP protocol. First, it extracts the network
 * topology (network nodes marked with the @networkNode NED property), regarding the
 * enabled (state=FORWARDING) links only. Then it analyzes the resulting graph
 * for connectedness and loop free-ness, using a modified depth-first search
 * with cycle detection. The results can be obtained with getter methods.
 */
// TODO: this module should be moved into the test folder somewhere
class INET_API StpTester : public cSimpleModule
{
  public:
    enum Color {
        WHITE, GRAY, BLACK
    };

  protected:
    bool loop = false;
    int numOfVisitedNodes = 0;
    int numOfNodes = 0;
    std::map<Topology::Node *, int> color;
    std::map<Topology::Node *, Topology::Node *> parent;
    Topology graph;

    simtime_t checkTime;
    cMessage *checkTimer = nullptr;

  public:
    // Includes network topology extraction
    StpTester();
    ~StpTester();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  protected:
    void dfsVisit(Topology::Node *node);
    bool isForwarding(Topology::Node *node, unsigned int portNum);

    // Analyzes the network graph
    void depthFirstSearch();

    // Getters for returning the result after a call to depthFirstSearch()
    bool isLoopFreeGraph();
    bool isConnectedGraph();
    bool isTreeGraph();

    int getNumOfNodes();
    int getNumOfVisitedNodes();
};

} // namespace inet

#endif // ifndef __INET_STPTESTER_H

