//
// This library is free software, you can redistribute it 
// and/or modify 
// it under  the terms of the GNU Lesser General Public License 
// as published by the Free Software Foundation; 
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU Lesser General Public License for more details.
//


//
// FIXME: add documentation here
//
simple RSVPAppl
    parameters:
        local_addr : string,
        dest_addr : string,
        traffic : string,
        isIR : bool,
        isER : bool,
        isSender : bool;
    
    gates:
        in: from_tcp;        
        in: from_rsvp;        
        in: from_mpls_switch;        
        in: from_tester;
        out: to_tcp;       
        out: to_rsvp;        
        out: to_mpls_switch;

endsimple
