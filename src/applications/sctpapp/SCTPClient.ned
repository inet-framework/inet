//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
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

package inet.applications.sctpapp;

simple SCTPClient like SCTPApp
{
    parameters:
        string address = default(""); // may be left empty ("")
        int port = default(0); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
        double startTime @unit(s) = default(1s); // time first session begins
        volatile int numRequestsPerSession = default(1);  // number of requests sent per session
        int numPacketsToReceive = default(0);
        int requestLength = default(1452); // length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // time gap between requests
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        double reconnectInterval @unit(s) = default(0);  // if connection breaks, waits this much before trying to reconnect
        int inboundStreams = default(17);
        int outboundStreams = default(1);
        int echoFactor = default(0);
        bool ordered = default(true);
        int queueSize = default(0);
        double stopTime @unit(s) = default(0s);
        double primaryTime @unit(s) = default(0s);
        string newPrimary = default("");
        bool finishEndsSimulation = default(false);
    gates:
        input sctpIn;
        output sctpOut;
}

