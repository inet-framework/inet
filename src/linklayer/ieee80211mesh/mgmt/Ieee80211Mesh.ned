//
// Copyright (C) 2008 Alfonso Ariza
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

package inet.linklayer.ieee80211mesh.mgmt;

import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;

simple Ieee80211Mesh like IIeee80211Mgmt
{
    parameters:
        @macRouting;  // Mac layer with routing protocol working in the level 2
        int frameCapacity = default(100);
        bool useProactive = default(true);
        bool useReactive = default(true);
        bool UseLwMpls = default(false);
        bool ProactiveFeedback = default(true);
        int maxHopProactiveFeedback = default(-1); // Maximun number of hops for to use the proactive feedback, not used yet, TO-DO
        int maxHopProactive = default(-1); // Maximun number of hops in the fix part of the network with the proactive feedback
        int maxHopReactive = default(-1); // Maximun number of hops by the reactive part for to use the proactive feedback, not used yet, TO-DO
        double maxDelay = default(0.1);
        int maxTTL = default(32); // the same that IP
        bool ETXEstimate = default(false);
        bool IsGateWay = default(false);
        double GateWayAnnounceInterval @unit("s") = default(100s);
        volatile double MacBroadcastDelay @unit("s") = default(uniform(0s,0.01s));
    //       int Willingness = default(3); //ms
    //       int Hello_ival = default(2); // sec
    //       int Tc_ival = default(5); // sec
    //       int Mid_ival = default(5); // sec
    //       int use_mac = default(0);
        @signal[enqueuePk](type=cMessage);
        @signal[dequeuePk](type=cMessage);
        @signal[dropPkByQueue](type=cMessage);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[dataQueueLen](type=long; interpolationmode=sample-hold);  // number of stored packets
    gates:
        input ETXProcIn;
        output ETXProcOut;
        input routingInProactive;
        output routingOutProactive;
        input routingInReactive;
        output routingOutReactive;
        input upperLayerIn @labels(Ieee802Ctrl/down);
        output upperLayerOut @labels(Ieee802Ctrl/up);
        input fromEthernet @labels(Ieee80211Frame);
        output toEthernet @labels(Ieee80211Frame);
        input macIn;
        output macOut;
        input agentIn;   // optional gates used for the agent in stations
        output agentOut; // optional gates used for the agent in stations
}
