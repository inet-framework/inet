//
//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


//
// Upper layers of TCP interfacing with the IP architecture.
//
// TCP application <-> TCP <-> TCP2IPInterface <-> IP
//
// author J.Reber, Sept. 2000
//
simple TCPApp
{
    parameters:
        bool debug;
        volatile double local_addr;
        volatile double server_addr;
        volatile double processing_time;
        volatile double timeout;
        volatile double appl_timeout;
        volatile double conn_ia_time;
        volatile double msg_ia_time;
        volatile double message_length;
    gates:
        input in;
        output out;
}
