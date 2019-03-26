/*
 * IPMapper.h
 *
 *  Created on: Apr 10, 2014
 *      Author: root
 */

#ifndef IPMAPPER_H_
#define IPMAPPER_H_

#include <string>
#include "inet/networklayer/common/L3AddressResolver.h"

using namespace std;

#include "inet/common/INETDefs.h"
namespace inet {
namespace greencloudsimulator {

class IPMapper {
    L3Address computingIP;
    L3Address srcIP;
    int allocatedTsk;

public:
    IPMapper();
    IPMapper( L3Address, int, L3Address );
    L3Address getComputingNodeIP() const;
    L3Address getSrcIP() const;
    int getTskID() const;
    void display() const;
    void setTskID(int);
    void setSrcID(L3Address);

    virtual ~IPMapper();
};

} /* namespace greencloudsimulator */
}
#endif /* IPMAPPER_H_ */
