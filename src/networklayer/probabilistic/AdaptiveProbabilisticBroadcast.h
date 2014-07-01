/*
 * ProbabilisticBroadcast.h
 *
 *  Created on: Nov 26, 2008
 *      Author: Damien Piguet, Dimitris Kotsakos, Jérôme Rousselot
 */

#ifndef __INET_ADAPTIVEPROBABILISTICBROADCAST_H
#define __INET_ADAPTIVEPROBABILISTICBROADCAST_H

#include <map>

#include "ProbabilisticBroadcast.h"
#include "Address.h"

namespace inet {

/**
 * @brief This class extends ProbabilisticBroadcast by adding
 *        an algorithm which adapts broadcasting parameters
 *        according to network conditions.
 *
 * @ingroup netwLayer
 * @author Dimitris Kotsakos, George Alyfantis, Damien Piguet
 **/
class INET_API AdaptiveProbabilisticBroadcast : public ProbabilisticBroadcast
{
public:
    AdaptiveProbabilisticBroadcast()
        : ProbabilisticBroadcast()
        , timeInNeighboursTable()
        , bvec()
        , neighMap()
    {}

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

protected:
    typedef std::map<Address, cMessage*> NeighborMap;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(cPacket* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMessage(cMessage* msg);

    void updateNeighMap(ProbabilisticBroadcastDatagram* m);

    void updateBeta();

    //read from omnetpp.ini
    simtime_t   timeInNeighboursTable; ///< @brief Default ttl for NeighborTable entries in seconds
    cOutVector  bvec;
    NeighborMap neighMap;
};

}


#endif
