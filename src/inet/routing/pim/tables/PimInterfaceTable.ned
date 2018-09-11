//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim.tables;

//
// The PimInterfaceTable module contains the PIM specific configuration
// parameters and state of the interfaces on which PIM is enabled.
//
// PIM routers must contain an instance of this module.
//
// The configuration file must contain <interface> elements under
// the document root with the following attributes:
//
// - @mode:          optional attribute, possible values are "dense" and "sparse".
//                   If missing then PIM is not used on that interface, otherwise
//                   it operates in the specified mode.
// - @state-refresh: optional boolean attribute, default is "false".
//                   If it is "true", then the router will send SR messages
//                   on this interface.
//
// Additionally each <interface> element may contain
// @hosts, @names, @towards, @among attributes to select the
// interfaces which the PIM parameters are applied to (see ~Ipv4NetworkConfigurator).
//
simple PimInterfaceTable
{
    parameters:
        @display("i=block/table");
        string interfaceTableModule;
        xml pimConfig = default(xml("<config/>"));
}

