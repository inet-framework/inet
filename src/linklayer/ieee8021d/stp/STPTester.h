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

#ifndef STPTESTER_H_
#define STPTESTER_H_

#include "INETDefs.h"
#include "Topology.h"
#include "STP.h"
#include <map>

class STPTester
{
    public:

        enum Color
        {
            WHITE, GRAY, BLACK
        };

    private:

        bool loop;
        int numOfVisitedNodes;
        int numOfNodes;
        std::map<Topology::Node *, int> color;
        std::map<Topology::Node *, Topology::Node *> parent;
        Topology graph;

        void dfsVisit(Topology::Node * node);
        bool isForwarding(Topology::Node * node, unsigned int portNum);
    public:
        STPTester();

        // Modified depth-first search with cycle detection
        void depthFirstSearch();

        bool isLoopFreeGraph();
        bool isConnectedGraph();
        bool isTreeGraph();

        int getNumOfNodes();
        int getNumOfVisitedNodes();
};

#endif /* STPTESTER_H_ */
