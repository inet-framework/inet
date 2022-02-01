//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
