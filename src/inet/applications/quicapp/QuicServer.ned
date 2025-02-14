//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.applications.quicapp;

import inet.applications.contract.IApp;

simple QuicServer like IApp
{
    parameters:
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
           
        volatile int expectedRcvDataSize @unit(byte) = default(0B); // amount of data that app is ready to read; 0 = infiniter, app is ready to read all avaliable data
        volatile double readInterval @unit(s)	= default(0ms);	// read interval by app
        
    	double stopTime @unit("s") = default(-1s);		// -1 = infiniter
    gates:
        input socketIn;
        output socketOut;
}
