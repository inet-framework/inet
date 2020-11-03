//
// Copyright:   (C) 2008-2010 CSEM SA, Neuchâtel, Switzerland.
//
//***************************************************************************
// file:        ProbabilisticBroadcast.ned
//
// author:      Damien Piguet
//
// description: Multi-hop ad-hoc data dissemination protocol based on
//              probabilistic broadcast: depending on application
//              parameters and network status, each packet will
//              go through a number of transmission attempts, each
//              with a certain probability. How number of transmissions
//              attempts and probability of forwarding are computed will
//              be derived from the simulations of this protocol.
//**************************************************************************
package inet.networklayer.probabilistic;

import inet.networklayer.base.NetworkProtocolBase;
import inet.networklayer.contract.INetworkProtocol;

//
// Multi-hop ad-hoc data dissemination protocol based on probabilistic broadcast.
//
// This method reduces the number of packets sent on the channel (reducing the
// broadcast storm problem) at the risk of some nodes not receiving the data.
// It is particularly interesting for mobile networks.
// Parameter maxNbBcast sets the maximum number of broadcast transmissions of a packet,
// parameter beta gives the transmission probability for each attempt,
// parameter bcperiod sets the time between two transmission attempts.
// Parameter maxFirstBcastBackoff sets a backoff window before first transmission attempt,
// and parameter timeInQueueAfterDeath sets the time during which a node remembers
// a packet whose time to live (TTL) expired, avoiding retransmitting it in case it comes back
// in the network because of some problem.
//
simple ProbabilisticBroadcast extends NetworkProtocolBase like INetworkProtocol
{
    parameters:
        @class(ProbabilisticBroadcast);
        int headerLength @unit(b) = default(96b); // length of the network packet header (in bits)
        // period between two probabilistic broadcast attempts
        double bcperiod @unit(s) = default(2 s);
        // probability of broadcast for each attempt
        double beta = default(0.8);
        // maximal number of broadcast attempts for each packet.
        int maxNbBcast = default(1);
        // maximal back-off before first broadcast attempt [seconds].
        // if don't want to use this parameter, simply set it
        // to a value that is greater than bcperiod.
        double maxFirstBcastBackoff @unit(s) = default(1 s);

        double timeToLive @unit(s) = default(10 s);
        // How many seconds the message should be kept in queue after its died.
        // That way the message is known if the node receives one of its
        // copy that isn't dead because of TTL de-synchronization due to
        // MAC backoff, propagation delay and clock drift.
        double timeInQueueAfterDeath @unit(s) = default(60 s);
}

