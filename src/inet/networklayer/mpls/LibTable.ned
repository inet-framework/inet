//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.mpls;

import inet.common.SimpleModule;

//
// Stores the LIB (Label Information Base), accessed by ~Mpls and its
// associated control protocols (~RsvpTe, ~Ldp) via direct C++ method calls.
//
// The config file must have the following format:
//
// The root element is <libtable>, which contains any number of
// <libentry> children. <libentry> represents an entry (row) in
// the table, and must have the following child elements:
// - <inLabel>: input label (integer)
// - <inInterface>: input interface (interface name, or "any" to match
//   regardless of the incoming interface, e.g. for ingress/PUSH entries)
// - <outInterface>: output interface (interface name)
// - <outLabel>: output label operations, see below.
//
// The <outLabel> element describes the output label operations in
// <op> child elements. <op> has two attributes:
// - code: accepted values are "push", "pop" and "swap".
// - value: label value for "push" (integer)
//
// A DTD is available in inet/etc/.
//
simple LibTable extends SimpleModule
{
    parameters:
        @class(LibTable);
        string interfaceTableModule;   // The path to the InterfaceTable module; used to resolve
                                        // the XML config's interface names to interface ids
        xml config = default(xml("<libtable/>")); // Table contents to be loaded on startup
        @display("i=block/table2");
}

