//
// Copyright (C) 2008 Irene Ruengeler
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

package inet.applications.sctpapp;

import inet.applications.contract.IApp;

simple SctpNatServer like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(0); // port number to listen on
        int numPacketsToSendPerClient = default(0);  // number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // length of a request (bytes)
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        int outboundStreams = default(1);
        int inboundStreams = default(1);
        bool ordered = default(true);
    gates:
        input socketIn;
        output socketOut;
}

