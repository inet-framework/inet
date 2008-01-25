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
// Acts as an adapter between \RSVP layer and network layer.
// It hides the encapsulation/decapsulation complexity from message
// processing.
//
// Operations
//
// RSVPInterface encapsulates packets passed from \RSVP layer and creates new
// packets which are consistent with Transport packet format. On the opposite
// way, the network packets are decapsulated into appropriate types of RSVP
// packets.
//
// Collaborations
//
// \RSVP-TE and network layer module are the module's collaborators. As
// mentioned above, the interactions focus on packet encapsulation and
// decapsulation.
//
simple RSVPInterface
{
    gates:
        input from_ip;
        input from_rsvp_app;
        output to_ip;
        output to_rsvp_app;

}
