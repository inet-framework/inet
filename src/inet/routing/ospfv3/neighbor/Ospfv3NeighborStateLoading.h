#ifndef __INET_OSPFV3NEIGHBORSTATELOADING_H_
#define __INET_OSPFV3NEIGHBORSTATELOADING_H_

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API Ospfv3NeighborStateLoading : public Ospfv3NeighborState
{
    /*
     * LSRs are sent to the neighbor for LSAs that have been discovered but not received.
     */
  public:
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::LOADING_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateLoading"); }
    ~Ospfv3NeighborStateLoading(){};
};

}//namespace inet

#endif

