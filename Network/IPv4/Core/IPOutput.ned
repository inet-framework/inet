// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


import "NF_IP_POST_ROUTING";


//
// IPOutput.
//
// Receive complete datagram from IPFragmentation:
// - throw away if ttl==0, otherwise send to IP OutputQueue
//
// author: Jochen Reber
//
module IPOutput 
    parameters: 
        hookName : string,
        procdelay : numeric const;
    gates: 
        in: fragmentationIn; 
        out: queueOut; 
        out: errorOut;
    submodules: 
        core: IPOutputCore; 
            parameters:
                procdelay = procdelay;
            display: "p=154,169;b=40,24,rect";
        queueHook: hookName like NF_IP_POST_ROUTING
            display: "p=152,82;b=40,24,rect";
    connections: 
        fragmentationIn --> core.fragmentationIn; 
        core.queueOut --> queueOut; 
        core.errorOut --> errorOut;
                                    
        core.netfilterOut --> queueHook.in; 
        queueHook.out --> core.netfilterIn; 
    display: "p=10,10;b=274,224,rect";
endmodule 

simple IPOutputCore 
    parameters: 
        procdelay: numeric const;
    gates: 
        in: fragmentationIn; 
        in: netfilterIn; 
        out: queueOut; 
        out: netfilterOut; 
        out: errorOut;
endsimple 

