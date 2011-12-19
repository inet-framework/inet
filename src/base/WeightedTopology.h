//
// Copyright (C) 2011 OpenSim Ltd
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
// @author: Zoltan Bojthe

#ifndef __INET_INETTOPOLOGY_H
#define __INET_INETTOPOLOGY_H


#include "INETDefs.h"

/**
 * expands cTopology with weighted shortest path finder algorithm
 */
class INET_API WeightedTopology : public cTopology
{
    public:
        /**
         * Constructor.
         */
        explicit WeightedTopology(const char *name=NULL) : cTopology(name) {};

        /**
         * Apply the Dijkstra algorithm to find all shortest paths to the given
         * graph node. The paths found can be extracted via Node's methods.
         * Uses weights in nodes and links.
         */
        void calculateWeightedSingleShortestPathsTo(Node *target);
};

#endif  // __INET_INETTOPOLOGY_H

