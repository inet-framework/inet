#ifndef __INET_OSPFV3NEIGHBORSTATEATTEMPT_H_
#define __INET_OSPFV3NEIGHBORSTATEATTEMPT_H_

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3NeighborStateAttempt : public Ospfv3NeighborState
{
    /*
     * Only for NBMA networks. No information received from neighbor but more concerted effort
     * should be made to establish connection. This is done by sending hello in hello intervals.
     */
  public:
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::ATTEMPT_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateAttempt"); }
    ~Ospfv3NeighborStateAttempt() {};
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3NEIGHBORSTATEATTEMPT_H_

