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
//


import
    "L2_EnqueueHook",
    "L2_DequeueHook",
    "Router_Deq_Hook_Trigger";
    

module Router_L2

    parameters:
        enq_hook_name:      string,
        deq_hook_name:      string,
        
        deq_hook_rate:      numeric;
        
        
    gates:
        in:  in;
        out: out;


    submodules:
    
            l2_enq_hook: enq_hook_name like L2_EnqueueHook;
        l2_deq_hook: deq_hook_name like L2_DequeueHook;
            
            deq_hook_trigger: Router_Deq_Hook_Trigger;
                parameters:
                    rate = deq_hook_rate;
    
                    
    connections:
    
            
                // enqueue
    
                l2_enq_hook.fromEnqueue <-- in;
                l2_enq_hook.toEnqueue   --> deq_hook_trigger.in;            // drop ack packets from enqueue hook
                
                
                // dequeue
                
                l2_deq_hook.fromDequeue <-- deq_hook_trigger.out;
                l2_deq_hook.toDequeue   --> deq_hook_trigger.deq_in;
                
                deq_hook_trigger.pck_out --> out;               
                
endmodule