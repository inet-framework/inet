// XXX FIXME: file shouldn't be needed according to manual 
//
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

simple Ospfd
    parameters:
        fsroot : string;
    gates:
        in: tcpIn;
        out: tcpOut;
        in: udpIn;
        out: udpOut;
        in: ipIn;
        out: ipOut;
endsimple
