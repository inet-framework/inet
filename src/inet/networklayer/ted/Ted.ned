//
// Copyright (C) 2005 Vojtech Janota
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

package inet.networklayer.ted;

//
// Traffic Engineering Database. Stores network topology with detailed
// link information, including total and reserved bandwidths.
//
// ~Ted is filled in and kept up-to-date via link state routing protocols,
// like ~LinkStateRouting or (in the future) OSPF-TE.
//
// Ted exposes several public methods for querying the current network state
// and allows ~RsvpTe and individual applications to calculate feasible LSPs
// meeting the chosen bandwidth criteria.
//
simple Ted
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        @display("i=block/table");
}

