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
// Implements \RSVP-TE, a signalling protocol for MPLS. The module
// processes \RSVP-TE messages, installs labels and does the reservation
// along LSP paths.
//
// Operations
//
// All RSVP messages are subclassed from RSVPPacket, and include
// RSVPPathMsg, RSVPPathTear, RSVPPathError, RSVPResvMsg, RSVPResvTear
// and RSVPResvError.
//
// Collaborations
//
// \RSVP-TE communicates with several components in the system:
// RSVPAppl, TED, OspfTe, and MPLSModule.
//
simple RSVP
    parameters:
        isIR : bool,
        isER : bool;
    gates:
        in: from_ip;
        in: from_rsvp_app;
        out: to_ip;
        out: to_rsvp_app;

endsimple
