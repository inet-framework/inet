//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_POLYHEDRONTEST_H
#define __INET_POLYHEDRONTEST_H

#include "inet/common/geometry/shape/polyhedron/Polyhedron.h"
#include "inet/common/geometry/common/Coord.h"
#include <vector>

namespace inet {

/*
 * This class takes a 3D point set, creates a Polyhedron with its point set
 * and then writes the faces and edges out to the standard output.
 */
class INET_API PolyhedronTest : public cSimpleModule
{
    protected:
        Polyhedron *polyhedron;
        std::vector<Coord> points;
        bool testWithConvexCombations;

    protected:
       virtual int numInitStages() const { return NUM_INIT_STAGES; }
       virtual void initialize(int stage);
       virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
       void parsePoints(const char *strPoints);
       void test() const;
       void printFaces() const;

    public:
        PolyhedronTest();
        virtual ~PolyhedronTest();
};

} /* namespace inet */

#endif // ifndef __INET_POLYHEDRONTEST_H
