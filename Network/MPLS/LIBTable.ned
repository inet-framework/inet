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
// Manages the label space and label mappings that belong to the current LSR.
//
// The LIB (Label Information Base) table has the following columns:
// - In-Lbl: Incoming Label
// - In-Intf: Incoming Interface
// - Out-Lbl: Outgoing Label
// - Out-Intf: Outgoing Interface
// - Opt: Operation code (PUSH=1, POP=2, or SWAP=3)
//
// For Edge routers, the incoming interface (Ingress \Router) or the outgoing interface
// (Egress \Router) might not be defined.
//
// In the simulation, LIB was built as a simple module. Each LSR has an LIB as a part
// of its system. When the LIB is first started up, it will load all of Label Switching
// Information from a local file. The name of this file is supplied as a parameter for LSR
// module.
//
// If the LIB file cannot be found, the LIB table is initialized empty. During the process
// of label requests and responses, new labels will be installed and added into this LIB.
// LIB TABLE module provides a set of utilities to manipulate and extract information
// from the LIB. Some examples are to get the outgoing Interface name and label pair
// for an incoming Interface and Label pair, and to install a new label when a Label
// Mapping message is received from upstream LSRs.
//
// Collaborations
//
// LIB module's collaborators are LDP and MPLS modules.
// The interaction between LIB and these modules are not through message exchange
// but the pointer to LIB module can be retried from the LDP and MPLS modules.
//
// Usages
//
// See <a href="lib-table-file.html">LIB table file format</a>.
//
simple LIBTable
    parameters:
        LibTableFileName : string,
        PrtTableFileName : string;
endsimple

