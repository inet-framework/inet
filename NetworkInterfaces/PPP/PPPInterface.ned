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
// \PPP interface. Complements the PPP module with an output queue
// for QoS and RED support.
//
// @see PPPInterfaceNoQueue
//
module PPPInterface
    parameters:
        queueType: string;
    gates:
        in: physIn;
        out: physOut;
        in: netwIn;
        out: netwOut;
    submodules:
        queue: queueType like OutputQueue;
            display: "i=block/queue;p=60,85;q=l2queue";
        ppp: PPP;
            parameters:
                queueModule = "queue",
                txQueueLimit = 1; // queue sends one packet at a time
            display: "i=block/rxtx;p=88,165";
    connections:
        netwIn --> queue.in display "m=n";
        queue.out --> ppp.netwIn;
        netwOut <-- ppp.netwOut display "m=n";
        physIn --> ppp.physIn display "m=s";
        physOut <-- ppp.physOut display "m=s";
endmodule


