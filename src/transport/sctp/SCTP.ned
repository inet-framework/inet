//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
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

package inet.transport.sctp;

import inet.transport.ISCTP;


//
// Implements the SCTP protocol.
//
simple SCTP like ISCTP
{
    parameters:
        //#====== SCTP Association Parameters =================================
        int numGapReports = default(3);
        double rtoInitial @unit(s) = default(3s);
        double rtoMin @unit(s) = default(1s);
        double rtoMax @unit(s) = default(60s);
        double rtoAlpha = default(0.125);
        double rtoBeta = default(0.250);
        int maxBurst = default(4);
        int assocMaxRetrans = default(10);
        int pathMaxRetrans = default(5);
        int maxInitRetrans = default(8);
        double sackPeriod @unit(s) = default(200ms);
        int sackFrequency = default(2);
        bool reactivatePrimaryPath = default(false);
        int sendQueueLimit = default(0);
        double validCookieLifetime @unit(s) = default(10s);

        //#====== Testing =====================================================

        //#====== Heartbeats ==================================================
        bool enableHeartbeats = default(true);
        double hbInterval @unit(s) = default(30s);

        //#====== Nagle Algorithm =============================================
        bool nagleEnabled = default(true);
        int naglePoint = default(1468);

        //#====== Congestion Control ==========================================
        bool fastRecoverySupported = default(true);
        string sctpAlgorithmClass = default("SCTPAlg");
        int ccModule = default(0);           // RFC4960=0

        int ssModule = default(0);           // ROUND_ROBIN=0
        int arwnd = default(65535);
        int swsLimit = default(3000);        // Limit for SWS
        bool udpEncapsEnabled = default(false);
        @display("i=block/wheelbarrow");

    gates:
        input from_appl[];
        input from_ip;
        input from_ipv6;
        output to_appl[];
        output to_ip;
        output to_ipv6;
}

