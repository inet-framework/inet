//
// Copyright (C) 2005 Vojta Janota
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


package inet.nodes.inet;

import inet.networklayer.extras.Dummy;


//
// A failed \IP router. Simply discards all packets it receives, and that's all.
//
module FailedRouter
{
    parameters:
        @node();
        int namid;
        string routingFile = default("");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        dummy: Dummy {
            gates:
                in[sizeof(pppg)];
                out[sizeof(pppg)];
                ethIn[sizeof(ethg)];
                ethOut[sizeof(ethg)];
        }
    connections allowunconnected:
        for i=0..sizeof(pppg)-1 {
            pppg$i[i] --> dummy.in[i];
            dummy.out[i] --> pppg$o[i];
        }
        for i=0..sizeof(ethg)-1 {
            ethg$i[i] --> dummy.ethIn[i];
            dummy.ethOut[i] --> ethg$o[i];
        }
}

