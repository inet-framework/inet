//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

//
// This module can be used to test Diffserv meter components.
// It is a traffic generator for the meter, and it also
// receives the packets from the meter. The different gates
// of the meter (corresponding to the conformance levels)
// should be connected to the input gates of MetersTestApp.
// The names of the conformance levels (colors) must be
// specified as a parameter.
//
// A report is written for each packet into resultFile.
//
simple MeterTestApp
{
  parameters:
    double startTime @unit(s) = default(0);
    double stopTime @unit(s) = default(0);
    int numPackets = default(0); // 0 = unlimited
    volatile double iaTime @unit(s);
    volatile int packetSize @unit(B);
    string colors; // specify here as many colors, as many input gate is connected (e.g. "green yellow red")
    string resultFile = default("result.txt");
  gates:
    output out;
    input in[];
}
