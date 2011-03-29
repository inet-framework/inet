//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.transport;

//
// Interface for SCTP protocol.
//
moduleinterface ISCTP
{
    @display("i=block/wheelbarrow");
    gates:
        input from_appl[];
        input from_ip;
        input from_ipv6;
        output to_appl[];
        output to_ip;
        output to_ipv6;
}

simple SCTP_None like ISCTP
{
    parameters:
        @display("i=old/x_noentry");
        @class("UnimplementedModule");
    gates:
        input from_appl[];
        input from_ip;
        input from_ipv6;
        output to_appl[];
        output to_ip;
        output to_ipv6;
}
