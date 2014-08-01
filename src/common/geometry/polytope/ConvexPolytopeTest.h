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

#ifndef __INET_CONVEXPOLYTOPETEST_H_
#define __INET_CONVEXPOLYTOPETEST_H_

#include "ConvexPolytope.h"
#include "Coord.h"
#include <vector>

namespace inet {

class ConvexPolytopeTest : public cSimpleModule
{
    protected:
        ConvexPolytope *polytope;
        std::vector<Coord> points;

    protected:
       virtual int numInitStages() const { return NUM_INIT_STAGES; }
       virtual void initialize(int stage);
       virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
       void parsePoints(const char *strPoints);
       void printFaces() const;

    public:
        ConvexPolytopeTest();
        virtual ~ConvexPolytopeTest();
};

} /* namespace inet */

#endif /* __INET_CONVEXPOLYTOPETEST_H_ */
