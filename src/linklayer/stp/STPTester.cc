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

#include "STPTester.h"
#include "IEEE8021DInterfaceData.h"

STPTester::STPTester()
{
    loop = false;
    numOfVisitedNodes = 0;
    graph.extractByProperty("node");
    numOfNodes = graph.getNumNodes();
}

void STPTester::depthFirstSearch()
{

    for (int i = 0; i < graph.getNumNodes(); i++)
    {
        color[graph.getNode(i)] = WHITE;
        parent[graph.getNode(i)] = NULL;
    }

    /*
     * for(int i = 0; i < graph.getNumNodes(); i++)
     *    if(color[graph.getNode(i) == WHITE)
     *       dfsVisit(graph.getNode(i));
     */

    // We only call dfsVisit() for a root node to test whether a graph is connected or not
    if (numOfNodes > 0)
        dfsVisit(graph.getNode(0));
}

void STPTester::dfsVisit(Topology::Node * node)
{
    color[node] = GRAY;

    for (int i = 0; i < node->getNumOutLinks(); i++)
    {
        Topology::LinkOut * linkOut = node->getLinkOut(i);
        Topology::Node * neighbor = linkOut->getRemoteNode();

        // If we found a port which is in state discarding,
        // then we do not expand this link

        if (!isForwarding(node, i))
            continue;

        // If we found a port that points to a remote port which is in state
        // discarding, then we also do not expand this link

        int remotePort = linkOut->getRemoteGate()->getIndex();
        if (!isForwarding(neighbor, remotePort))
            continue;

        if (color[neighbor] == WHITE)
        {
            parent[neighbor] = node;
            dfsVisit(neighbor);
        }

        if (color[neighbor] == GRAY && parent[node] != neighbor)
            loop = true;
    }
    color[node] = BLACK;
    numOfVisitedNodes++;
}

bool STPTester::isLoopFreeGraph()
{
    return !loop;
}

bool STPTester::isConnectedGraph()
{
    return numOfNodes == numOfVisitedNodes;
}

bool STPTester::isTreeGraph()
{
    return !loop && (numOfNodes == numOfVisitedNodes);
}

int STPTester::getNumOfNodes()
{
    return numOfNodes;
}

int STPTester::getNumOfVisitedNodes()
{
    return numOfVisitedNodes;
}

bool STPTester::isForwarding(Topology::Node * node, unsigned int portNum)
{
    cModule * tmpIfTable = node->getModule()->getSubmodule("interfaceTable");
    IInterfaceTable * ifTable = dynamic_cast<IInterfaceTable*>(tmpIfTable);

    // EtherHost has no InterfaceTable
    if (ifTable == NULL)
        return true;

    cGate * gate = node->getModule()->gate("ethg$o", portNum);
    InterfaceEntry * gateIfEntry = ifTable->getInterfaceByNodeOutputGateId(gate->getId());
    IEEE8021DInterfaceData * portData = gateIfEntry->ieee8021DData();

    // If portData does not exist, then it implies that
    // the node is not a switch
    if (portData == NULL)
        return true;

    return portData->isForwarding();
}
