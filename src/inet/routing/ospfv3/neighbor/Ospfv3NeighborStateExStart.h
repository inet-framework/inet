#ifndef __INET_OSPFV3NEIGHBORSTATEEXSTART_H_
#define __INET_OSPFV3NEIGHBORSTATEEXSTART_H_

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborStateExStart : public Ospfv3NeighborState
{
    /*
     * First step in creating adjacency. A decision is made which of the two routers will be the master
     * and which will be the slave. Initial DD sequence number is decided. Neighbor conversations in
     * this or higher states are called adjacencies.
     */
  public:
    ~Ospfv3NeighborStateExStart() {}
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::EXCHANGE_START_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateExStart"); }
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3NEIGHBORSTATEEXSTART_H_

