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
    parameters:
    debug       : bool,
        local_addr  : numeric,
        server_addr : numeric,
        processing_time : numeric,
        timeout     : numeric,
        appl_timeout    : numeric,
        conn_ia_time    : numeric,
        msg_ia_time : numeric,
        message_length  : numeric;
    gates:
        in: in;
        out: out;
endsimple
