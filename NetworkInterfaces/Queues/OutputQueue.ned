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



import   
    "L2_EnqueueHook", 
    "L2_DequeueHook"; 
                      
 

//
// Enqueue Module of the Output Queue
//
// Responsibilities:
// - when receives IPDatagram from IP layer: sends it to EnqueueHook
// - when receives IPDatagram or Discard_Packet message from EnqueueHook:
//   ignores Discard_Packet-Message
// - on IPDatagram: notify Dequeue directly with WakeUp_Queue message
//
simple Enqueue 
    parameters:
        procdelay : numeric const;
    gates: 
        in: fromIP; 
        in: fromEnqHook; 
        in: processorManagerIn;
        out: toEnqHook; 
endsimple 


//
//  receive Networkinterface Idle-Message from Network Interface:
//      set nwi_idle = true
//
//  receive Wakeup_Queue message from directIn gate:
//      set queue_empty = false
//
//  ==> if  nwi_idle && !queue_empty:
//      send Request_Packet-Message to Dequeue-Hook
//
//  receive IPDatagram from DequeueHook:
//  send IPDatagram to Network Interface,
//  set nwi_idle = false
//
//  receive NoPacket-Message from DequeueHook:
//  set queue_empty = true
//
//  release Kernel
//
simple Dequeue 
    parameters:
        procdelay : numeric const;
    gates: 
        in: fromNW; 
        in: fromDeqHook; 
        in: processorManagerIn;
        in: inDirekt; 
        out: toNW; 
        out: toDeqHook; 
endsimple 

//
// author: Jochen Reber
//
module OutputQueue 
    parameters:
        enqueueHookName : string,
        dequeueHookName : string,
        enqueue_delay : numeric,
        dequeue_delay : numeric const;
    gates: 
        in: fromIP; 
        in: fromNW; 
        out: toNW; 
    submodules: 
        enq: Enqueue; 
            parameters:
                procdelay = enqueue_delay;
            display: "p=57,46;b=40,24,rect";
        deq: Dequeue; 
            parameters:
                procdelay = dequeue_delay;
            display: "p=58,118;b=40,24,rect";
        enqHook: enqueueHookName like L2_EnqueueHook; 
            display: "p=167,39;b=40,24,rect";
        deqHook: dequeueHookName like L2_DequeueHook; 
            display: "p=167,116;b=40,24,rect";
    connections nocheck: 
        fromIP --> enq.fromIP; 
                               
        fromNW --> deq.fromNW; 
        toNW <-- deq.toNW; 
                           
        enq.toEnqHook --> enqHook.fromEnqueue; 
        enq.fromEnqHook <-- enqHook.toEnqueue; 
                                               
        deq.toDeqHook --> deqHook.fromDequeue; 
        deq.fromDeqHook <-- deqHook.toDequeue; 
    display: "p=10,10;b=205,135,rect";
endmodule 
