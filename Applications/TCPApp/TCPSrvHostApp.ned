//
// Copyright (C) 2004 Andras Varga
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


//
// This module hosts TCP-based server applications. It dynamically creates
// and launches a new module for each incoming connection.
//
// The type of modules to create should be specified in the serverProcess
// parameter. The module type to be used should be subclassed from the
// TCPServerProcess C++ class.
//
// The module will get messages from TCP via the TCPSocket callback interface,
// and can send messages via the TCPSocket send method. The OMNeT++ send()
// method is not used.
//
// IMPORTANT: Before you try to use this module, make sure you actually need it!
// In most cases, TCPGenericSrvApp and GenericAppMsg will be completely 
// enough, and they are a lot easier to handle. You'll want to subclass your
// client from TCPGenericCliAppBase then.
//
simple TCPSrvHostApp
    parameters:
        address: string, // may be left empty ("")
        port: numeric const, // port number to listen on
        serverThread : string; // type (class name) of modules to launch on incoming connections
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

