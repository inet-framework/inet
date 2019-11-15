#ifndef __INET_OSPFV3NEIGHBORSTATELOADING_H_
#define __INET_OSPFV3NEIGHBORSTATELOADING_H_

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborStateLoading : public Ospfv3NeighborState
{
    /*
     * LSRs are sent to the neighbor for LSAs that have been discovered but not received.
     */
  public:
    ~Ospfv3NeighborStateLoading() {}
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::LOADING_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateLoading"); }
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3NEIGHBORSTATELOADING_H_

