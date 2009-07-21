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


package inet.applications.tcpapp;

//
// Template for TCP applications. It shows what gates a TCP app
// needs, to be able to be used in StandardHost etc.
//
moduleinterface TCPApp
{
    parameters:
        @display("i=block/app");
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

