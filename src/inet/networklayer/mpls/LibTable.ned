//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

