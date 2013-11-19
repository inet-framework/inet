//***************************************************************************
// file:        ProbabilistcBroadcast.ned
//
// author:      Damien Piguet, Dimitris Kotsakos, Jérôme Rousselot
// 
// copyright:   (C) 2008 CSEM SA, Neuchâtel, Switzerland.
//
// description: Multi-hop ad-hoc data dissemination protocol base on
//              probabilistic broadcast: depending on application
//              parameters and network status, each packet will
//              go through a number of transmission attempts, each
//              with a certain probability. How number of transmissions
//              attempts and probability of forwarding are computed will
//              be derived from the simulations of this protocol. 
//**************************************************************************

package inet.networklayer.probabilistic;

//
// Multi-hop ad-hoc data dissemination protocol based on probabilistic broadcast, with adaptive parameters.
//
// This protocol performs network-level broadcast using a probabilistic mechanism.
// This method reduces the number of packets sent on the channel (reducing the
// broadcast storm problem) at the risk of some nodes not receiving the data.
// It is particularly interesting for mobile networks.
// This version of probabilistic broadcast automatically adapts transmission
// probabilities depending on the estimated number of neighbours.
// timeInNeighboursTable is its unique parameter, and sets the time
// during which the node remembers a neighbour. It is particularly important
// in mobile networks: in that case, the faster the nodes, the smaller the
// timeInNeighboursTable parameter.
//
simple AdaptiveProbabilisticBroadcast extends ProbabilisticBroadcast
{
    parameters:
        @class(AdaptiveProbabilisticBroadcast);
        double timeInNeighboursTable @unit(s) = default(60 s);
}
