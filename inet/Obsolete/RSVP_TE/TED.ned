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
// Traffic Engineering Database.
//
// Some extensions to the IGP, for example OSPF-TE, allow associate link state
// database to include the link specific resource information. The information
// about resource availability and usage is important for the constraint based
// resource algorithm to find optimal path.
//
// Operations
//
// Traffic Engineering Database (TED) is only a database itself. No messages
// exchanged with other modules.
//
// Collaborations
//
// Each LSR holds a local copy of TED. In reality, these local copies may be
// different when the convergence has not been completed. In the simulation,
// these local copies are exactly the same as they share the same pointer.
//
// The modules that will extract information from TED are OSPF-TE and \RSVP-TE
// modules. As mentioned above, an important property of TED is that, as is
// true with the link state database, the TED has a consistent view from all
// the routers in the same area. Since the message processing has not been
// implemented for OSPF-TE in this simulation version, a common TED is shared
// for all routers in a network. The judgment for this is as time delay is not
// a key issue in this implementation; the database convergence period can be
// ignored without any effects on the simulation result.
//
// Usages
//
// The TED module detects the links between modules of types listed in the
// moduleTypes parameter; it should be set to e.g. "RSVP_LSR StandardHost".
//
simple TED
{
    parameters:
        string moduleTypes;
        @display("i=block/table");
}


