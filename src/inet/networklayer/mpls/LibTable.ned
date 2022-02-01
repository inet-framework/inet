//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.mpls;

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
// - <inInterface>: input interface (interface name)
// - <outInterface>: output interface (interface name)
// - <outLabel>: output label operations, see below.
// - <color>: color (integer); this element is optional.
//
// The <outLabel> element describes the output label operations in
// <op> child elements. <op> has two attributes:
// - code: accepted values are "push", "pop" and "swap".
// - value: label value for "push" (integer)
//
// A DTD is available in inet/etc/.
//
simple LibTable
{
    parameters:
        xml config = default(xml("<libtable/>")); // table contents to be loaded on startup
        @display("i=block/table2");
}

