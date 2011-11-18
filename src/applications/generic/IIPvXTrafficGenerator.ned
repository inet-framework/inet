//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.applications.generic;

//
// Prototype for modules that generate traffic directly over IP.
// Compatible with both ~IPv4 and ~IPv6.
//
// @see ~IPvXTrafGen, ~IPvXTrafSink
//
moduleinterface IIPvXTrafficGenerator
{
    @display("i=block/app");
    gates:
        input ipIn @labels(IPv4ControlInfo/up);
        input ipv6In @labels(IPv6ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down);
        output ipv6Out @labels(IPv6ControlInfo/down);
}

