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
// All RSVP messages are subclassed from RSVPMessage, and include
// RSVPPathMsg, RSVPPathTear, RSVPPathError, RSVPResvMsg, RSVPHelloMsg.
//
// Collaborations
//
// \RSVP-TE communicates with the following components in the system:
// TED, MPLSModule, and Scenario.
//
simple RSVP
	parameters:
		traffic : xml,
		peers : string,
		helloInterval : numeric,
		helloTimeout : numeric;
	gates:
		in: from_ip;
		out: to_ip;
		in: from_mpls_switch;
		in: from_app;
endsimple
