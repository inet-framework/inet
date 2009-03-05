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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.transport.sctp;

simple SCTP //
{
        parameters:
        int numGapReports = default(3);
        double rtoMin @unit(s) = default(1s);
        double rtoMax @unit(s) = default(60s);
        double rtoInitial @unit(s) = default(3s);
        double rtoAlpha @unit(s) = default(125ms);
        double rtoBeta @unit(s) = default(250ms);
        int maxBurst = default(4);
        double validCookieLifetime @unit(s) = default(10s);
        int assocMaxRetrans = default(10);
        int pathMaxRetrans = default(5);
        int maxInitRetrans = default(8);
        double hbInterval @unit(s) = default(30s);
        double sackPeriod @unit(s) = default(200ms);
        int sackFrequency = default(2);
        string sctpAlgorithmClass = default("SCTPAlg"); 
        bool nagleEnabled = default(true);
        int naglePoint = default(1468);
        bool reactivatePrimaryPath = default(false);
        int fragPoint = default(1400);
        bool fragment = default(false);
        int sendQueueLimit = default(0);
        int ccModule = default(0); // RFC4960=0
	int ssModule = default(0); //ROUND_ROBIN=0
        int arwnd = default(65535);
	int swsLimit = default(3000); //Limit for SWS
    gates:
        input from_appl[];
        input from_ip;
        input from_ipv6;
        output to_appl[];
        output to_ip;
        output to_ipv6;
}
