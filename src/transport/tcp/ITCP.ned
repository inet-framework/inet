//
// Copyright (C) 2004 Andras Varga
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


package inet.transport.tcp;

//
// Interface for TCP protocol implementations. All TCP implementations should
// implement this (i.e. TCP like ITCP)
//
moduleinterface ITCP
{
        @display("i=block/wheelbarrow");
    gates:
        input appIn[];
        input ipIn;
        input ipv6In;
        output appOut[];
        output ipOut;
        output ipv6Out;
}
