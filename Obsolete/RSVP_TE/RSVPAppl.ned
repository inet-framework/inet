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
// Reads traffic.xml file (only if Ingress router!) and stores
// it into a traffic_request_t[] array. Then ...
//# FIXME
//
simple RSVPAppl
    parameters:
        traffic : string, // traffic XML file
        isIR : bool,
        isER : bool;
    gates:
        //# FIXME seems to be redundant: in: from_tcp;
        in: from_rsvp;
        in: from_mpls_switch; // for receiving path request commands from MPLSModule via sendDirect()
        in: from_tester; // for receiveing commands from Tester via sendDirect()
        //# FIXME seems to be redundant: out: to_tcp;
        out: to_rsvp;
        //# FIXME seems to be unused: out: to_mpls_switch;

endsimple
