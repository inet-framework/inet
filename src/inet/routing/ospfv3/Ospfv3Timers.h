#ifndef __INET_OSPFV3TIMERS_H_
#define __INET_OSPFV3TIMERS_H_

namespace inet {
namespace ospfv3 {

enum Ospfv3Timers
{
    INIT_PROCESS = 0,
    HELLO_TIMER = 1,
    WAIT_TIMER = 2,
    NEIGHBOR_INACTIVITY_TIMER = 3,
    NEIGHBOR_POLL_TIMER = 4,
    NEIGHBOR_DD_RETRANSMISSION_TIMER = 5,
    NEIGHBOR_UPDATE_RETRANSMISSION_TIMER = 6,
    NEIGHBOR_REQUEST_RETRANSMISSION_TIMER = 7,
    ACKNOWLEDGEMENT_TIMER = 8,
    DATABASE_AGE_TIMER = 9,
    HELLO_TIMER_INIT = 10 // timer set by process, for initialisation of hello msgs
};

} //namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3TIMERS_H_

