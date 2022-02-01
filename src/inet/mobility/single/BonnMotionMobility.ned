//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Uses the native file format of BonnMotion.
//
// The file is a plain text file, where every line describes the motion
// of one host. A line consists of one or more (t, x, y, [z]) tuples of real
// numbers, like:
//
// t1 x1 y1 t2 x2 y2 t3 x3 y3 t4 x4 y4 ...
//
// or
//
// t1 x1 y1 z1 t2 x2 y2 z2 t3 x3 y3 z3 t4 x4 y4 z4...
//
// The meaning is that the given node gets to (xk,yk) at tk. There's no
// separate notation for wait, so x and y coordinates will be repeated there.
//
simple BonnMotionMobility extends MovingMobilityBase
{
    parameters:
        bool is3D = default(false); // whether the trace file contains triplets or quadruples
        string traceFile; // the BonnMotion trace file
        int nodeId; // selects line in trace file; -1 gets substituted to parent module's index
        @class(BonnMotionMobility);
}

