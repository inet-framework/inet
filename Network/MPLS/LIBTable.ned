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
// Manages the label space and label mappings that belong to a Label Switched
// \Router (LSR). Every LSR model must contain a LIBTable module with the name
// libTable.
//
// The LIB (Label Information Base) table has the following columns:
//   - In-Lbl: Incoming Label
//   - In-Intf: Incoming Interface
//   - Out-Lbl: Outgoing Label
//   - Out-Intf: Outgoing Interface
//   - Opt: Operation code (PUSH=1, POP=2, or SWAP=3)
//
// For Edge routers, the incoming interface (Ingress \Router) or the outgoing interface
// (Egress \Router) might not be defined.
//
// Upon startup, the module initializes the table from the optional LIB and PRT
// table files. The formats of these files are described on the page
// <a href="lib-table-file.html">LIB table file format</a>.
//
// During the process of label requests and responses, new labels will be installed
// and added into this LIB. This module exposes C++ methods to manipulate the LIB
// and extract information from it. Some examples are getting the (outgoing interface,
// label) pair for an (incoming Interface, label) pair, and installing a new label
// when a Label Mapping message is received from upstream LSRs.
//
// Collaborations
//
// LIB is accessed from MPLSModule and signalling (NewLDP, RSVPAppl) modules,
// by calling its public C++ methods.
//
simple LIBTable
    parameters:
        LibTableFileName : string, // file name or ""
        PrtTableFileName : string; // file name or ""
endsimple

