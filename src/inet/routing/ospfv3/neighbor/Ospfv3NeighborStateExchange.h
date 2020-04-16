#ifndef __INET_OSPFV3NEIGHBORSTATEEXCHANGE_H_
#define __INET_OSPFV3NEIGHBORSTATEEXCHANGE_H_

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborStateExchange : public Ospfv3NeighborState
{
    /*
     * DDs are sent to the neighbor. Each DD has DD sequence number and is explicitly acknowledged.
     * Link state requests may be sent in this state. Flooding procedure works on all adjacencies.
     */
  public:
    ~Ospfv3NeighborStateExchange() {}
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::EXCHANGE_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateExchange"); }
};

} // namespace ospfv3
} //namespace inet

#endif // __INET_OSPFV3NEIGHBORSTATEEXCHANGE_H_

