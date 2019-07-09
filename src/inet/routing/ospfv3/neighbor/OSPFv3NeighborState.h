#ifndef __INET_OSPFV3NEIGHBORSTATE_H_
#define __INET_OSPFV3NEIGHBORSTATE_H_

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/common/INETDefs.h"


namespace inet{

class INET_API OSPFv3NeighborState
{
  protected:
    void changeState(OSPFv3Neighbor *neighbor, OSPFv3NeighborState *newState, OSPFv3NeighborState *currentState);

  public:
    virtual void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) = 0;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const = 0;
    virtual std::string getNeighborStateString() = 0;
    virtual ~OSPFv3NeighborState(){};
};

}//namespace inet
#endif
