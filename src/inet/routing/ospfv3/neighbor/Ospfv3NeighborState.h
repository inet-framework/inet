#ifndef __INET_OSPFV3NEIGHBORSTATE_H_
#define __INET_OSPFV3NEIGHBORSTATE_H_

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborState
{
  protected:
    void changeState(Ospfv3Neighbor *neighbor, Ospfv3NeighborState *newState, Ospfv3NeighborState *currentState);

  public:
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) = 0;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const = 0;
    virtual std::string getNeighborStateString() = 0;
    virtual ~Ospfv3NeighborState() {};
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3NEIGHBORSTATE_H_

