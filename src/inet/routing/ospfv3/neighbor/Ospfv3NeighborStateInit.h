#ifndef __INET_OSPFV3NEIGHBORSTATEINIT_H_
#define __INET_OSPFV3NEIGHBORSTATEINIT_H_

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborStateInit : public Ospfv3NeighborState
{
    /*
     * Hello has recently been seen from the neighbor, but bidirectional comm has not been established.
     * This means that the router has not seen itself in the Hello from neighbor. All neighbors
     * in this or higher states are listed in hello.
     */
  public:
    ~Ospfv3NeighborStateInit() {}
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::INIT_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateInit"); }
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3NEIGHBORSTATEINIT_H_

