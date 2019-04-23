/*
 * IPMapper.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: root
 */

#include "IPMapper.h"

namespace inet {
namespace greencloudsimulator {

IPMapper::IPMapper() {
    // TODO Auto-generated constructor stub

}
IPMapper::IPMapper(L3Address cip, int tid, L3Address sip)
{
        computingIP = cip;
        allocatedTsk =tid;
        srcIP=sip;

}
void IPMapper::setTskID(int id)
{
    allocatedTsk = id;
}
void IPMapper::setSrcID(L3Address addr)
{
    srcIP= addr;

}

L3Address IPMapper::getComputingNodeIP() const
{
    return computingIP;
}
L3Address IPMapper::getSrcIP() const
{
    return srcIP;

}
int IPMapper::getTskID() const
{
    return allocatedTsk;
}

void IPMapper::display() const
{
    EV_INFO <<"Computing Node IP:"<<computingIP<<" \t "<<" User IP "<<srcIP<<" \t "<< "Assigned task: "<<allocatedTsk<<endl;
}
IPMapper::~IPMapper() {
    // TODO Auto-generated destructor stub
}

}
} /* namespace greencloudsimulator */
