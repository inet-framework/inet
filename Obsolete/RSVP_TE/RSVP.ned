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
// Implements \RSVP-TE. The module processes \RSVP-TE messages, installs
// labels and does the reservation along LSP paths.
//
// Operations
//
// Message formats and message processing can be referred at section 7.4 and
// 7.5 of this report. FIXME resolve ref!
//
// Collaborations
//
// \RSVP-TE is expected to communicate with different components in the system.
// These components are RSVPInterface module, TED module, OSPF-TE module, and
// MPLS module.
//
// Being a signaling protocol like LDPProc, \RSVP-TE is implemented as a simple
// module. It has buffers to store Label Request messages when finding the
// paths for FEC in these messages. \RSVP-TE communicates directly with MPLS
// Module without via any transmission links. There are three types of
// messages exchanged between these two modules:
// - Messages of label request for unknown FEC from MPLS module to
//   \RSVP-TE module
// - Messages of label replied for a particular FEC from \RSVP-TE module to
//   MPLS module
// - Messages of ready state signaling from \RSVP-TE module to MPLS module to
//   signal the MPLS module that the singalling protocol has passed the
//   initialization stage.
//
// Usages
//
// \RSVP-TE module needs the input of the LSR loopback address. In addition,
// for TE application, it requires inputs stored in an XML file name
// "traffic.xml", see <a href="traffic-xml-file.html">The traffic.xml File</a>.
// The file must be in the same folder as the executable network
// simulation file.
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
