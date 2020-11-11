#ifndef __INET_OSPFV3NEIGHBORSTATEDOWN_H
#define __INET_OSPFV3NEIGHBORSTATEDOWN_H

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborStateDown : public Ospfv3NeighborState
{
    /*
     * Indicates that no Hello has been received. On NBMA networks, hello packets may still
     * be sent to Down neighbors but at a reduced rate.
     */

  public:
    ~Ospfv3NeighborStateDown() {}
    virtual void processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::DOWN_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateDown"); }
};

} // namespace ospfv3
} // namespace inet

#endif

