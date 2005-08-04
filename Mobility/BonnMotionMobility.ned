//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Uses the native file format of BonnMotion (http://www.cs.uni-bonn.de/IV/BonnMotion/).
//
// The file is a plain text file, where every line describes the motion
// of one host. A line consists of one or more (t, x, y) triplets of real
// numbers, like:
//
// t1 x1 y1 t2 x2 y2 t3 x3 y3 t4 x4 y4 ...
//
// The meaning is that the given node gets to (xk,yk) at tk. There's no
// separate notation for wait, so x and y coordinates will be repeated there.
//
// @author Andras Varga
//
simple BonnMotionMobility
    parameters:
        debug : bool, // debug switch
        traceFile: string, // the BonnMotion trace file
        nodeId: numeric const, // selects line in trace file; -1 gets substituted to parent module's index
        updateInterval : numeric const; // time interval to update the hosts position
endsimple



