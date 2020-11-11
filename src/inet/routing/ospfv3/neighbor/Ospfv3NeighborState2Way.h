#ifndef __INET_OSPFV3NEIGHBORSTATE2WAY_H
#define __INET_OSPFV3NEIGHBORSTATE2WAY_H

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborState2Way : public Ospfv3NeighborState
{
    /*
     * Bidirectional communication established. Highest state before exchanging databases.
     * DR and BDR are chosen during this state.
     */

  public:
    ~Ospfv3NeighborState2Way() {}
    virtual void processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::TWOWAY_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborState2Way"); }
};

} // namespace ospfv3
} // namespace inet

#endif

