#ifndef __INET_OSPFV3NEIGHBORSTATELOADING_H_
#define __INET_OSPFV3NEIGHBORSTATELOADING_H_

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API OSPFv3NeighborStateLoading : public OSPFv3NeighborState
{
    /*
     * LSRs are sent to the neighbor for LSAs that have been discovered but not received.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::LOADING_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateLoading");};
    ~OSPFv3NeighborStateLoading(){};
};

}//namespace inet
#endif
