//
// This library is free software, you can redistribute it 
// and/or modify 
// it under  the terms of the GNU Lesser General Public License 
// as published by the Free Software Foundation; 
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU Lesser General Public License for more details.
//


//
// This module is dynamically instantiated by LDPInterface.
// FIXME should be renamed to LDPConnHandler or something like this. 
//
// Attempts to provide a simplified TCP interface for the LDP protocol:
// just establishes the connection (FIXME no error handling!) 
// and relays messages in both directions. 
//
// FIXME really needs a rewrite.
// 
simple TCPClientProc
{
    gates:
        input from_ldp;
        input from_tcp;
        output to_ldp;
        output to_tcp;
}
