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

#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/linklayer/ieee8021d/tester/StpTester.h"

namespace inet {

Define_Module(StpTester);

StpTester::StpTester()
{
}

StpTester::~StpTester()
{
    cancelAndDelete(checkTimer);
}

void StpTester::initialize()
{
    checkTimer = new cMessage("checktime");
    checkTime = par("checkTime");
    scheduleAfter(checkTime, checkTimer);
}

void StpTester::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        depthFirstSearch();
        if (isLoopFreeGraph())
            EV_DEBUG << "The network is loop-free" << endl;
        else
            EV_DEBUG << "The network is not loop-free" << endl;
        if (isConnectedGraph())
            EV_DEBUG << "All nodes are connected with each other" << endl;
        else
            EV_DEBUG << "Not all nodes are connected with each other" << endl;
        if (isTreeGraph())
            EV_DEBUG << "The network topology is a tree topology" << endl;
        scheduleAfter(checkTime, msg);
    }
    else {
        throw cRuntimeError("This module only handle selfmessages");
    }
}

void StpTester::depthFirstSearch()
{
    loop = false;
    numOfVisitedNodes = 0;
    graph.extractByProperty("networkNode");
    numOfNodes = graph.getNumNodes();

    for (int i = 0; i < graph.getNumNodes(); i++) {
        color[graph.getNode(i)] = WHITE;
        parent[graph.getNode(i)] = nullptr;
    }

    /* Use this for testing loop-freeness in all connected components.
     * Note that in this case the algorithm is unable to decide
     * whether a graph is connected.
     *
     * for (int i = 0; i < graph.getNumNodes(); i++)
     *    if (color[graph.getNode(i)] == WHITE)
     *       dfsVisit(graph.getNode(i));
     */

    // We only call dfsVisit() for a root node to test whether a graph is connected or not
    if (numOfNodes > 0)
        dfsVisit(graph.getNode(0));
}

void StpTester::dfsVisit(Topology::Node *node)
{
    color[node] = GRAY;

    for (int i = 0; i < node->getNumOutLinks(); i++) {
        Topology::LinkOut *linkOut = node->getLinkOut(i);
        Topology::Node *neighbor = linkOut->getRemoteNode();

        // If we found a port which is in state discarding,
        // then we do not expand this link

        int localGateIndex = node->getModule()->isGateVector("ethg$o") ? i : -1;
        if (!isForwarding(node, localGateIndex))
            continue;

        // If we found a port that points to a remote port which is in state
        // discarding, then we also do not expand this link

        auto remoteGate = linkOut->getRemoteGate();
        int remotePort = remoteGate->isVector() ? remoteGate->getIndex() : -1;
        if (!isForwarding(neighbor, remotePort))
            continue;

        if (color[neighbor] == WHITE) {
            parent[neighbor] = node;
            dfsVisit(neighbor);
        }

        if (color[neighbor] == GRAY && parent[node] != neighbor)
            loop = true;
    }
    color[node] = BLACK;
    numOfVisitedNodes++;
}

bool StpTester::isLoopFreeGraph()
{
    return !loop;
}

bool StpTester::isConnectedGraph()
{
    return numOfNodes == numOfVisitedNodes;
}

bool StpTester::isTreeGraph()
{
    return !loop && (numOfNodes == numOfVisitedNodes);
}

int StpTester::getNumOfNodes()
{
    return numOfNodes;
}

int StpTester::getNumOfVisitedNodes()
{
    return numOfVisitedNodes;
}

bool StpTester::isForwarding(Topology::Node *node, unsigned int portNum)
{
    cModule *tmpIfTable = node->getModule()->getSubmodule("interfaceTable");
    IInterfaceTable *ifTable = dynamic_cast<IInterfaceTable *>(tmpIfTable);

    // EtherHost has no InterfaceTable
    if (ifTable == nullptr)
        return true;

    cGate *gate = node->getModule()->gate("ethg$o", portNum);
    InterfaceEntry *gateIfEntry = CHK(ifTable->findInterfaceByNodeOutputGateId(gate->getId()));
    Ieee8021dInterfaceData *portData = gateIfEntry->findProtocolData<Ieee8021dInterfaceData>();

    // If portData does not exist, then it implies that
    // the node is not a switch
    if (portData == nullptr)
        return true;

    return portData->isForwarding();
}

} // namespace inet

