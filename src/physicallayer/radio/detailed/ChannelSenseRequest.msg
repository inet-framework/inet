
cplusplus {{
#include "ChannelState.h"
}}

class noncobject ChannelState;

enum SenseMode {
    UNTIL_IDLE = 1;		//sense until channel is busy or timeout is reached
    UNTIL_BUSY = 2;		//sense until channel is idle or timeout is reached
    UNTIL_TIMEOUT = 3;	//sense until timeout is reached
};
    
//
// The ChannelSenseRequest is a control packet from the
// MAC layer to the physical layer. In contrast to the
// "getChannelState()" method of the MacToPhyInterface
// which imediatly returns the current ChannelState
// the ChannelSenseRequest tells the physical layers decider
// to sense over a period of time before sending back
// the request together with the ChannelState over this period.
//
// Every ChannelSenseRequest sent to the physical layer
// has to define a SenseMode and a timeout.
// The timeout defines after how much seconds the request
// should be sent back to the MAC layer at latest.
// The SenseMode tells the physical layers the purpose of the
// sense. The SenseModes "UNTIL_IDLE" and "UNTIL_BUSY" are
// telling it to inform the MAC layer as soon as the 
// channel turns idle/busy by sending the request back to it.
// If the channel state does not change the request is sent
// back after the timeout.
// While "UNTIL_IDLE" and "UNTIL_BUSY" can be used as pure
// conveniance functions which don't necessarily represent a
// real channel sense, "UNTIL_TIMEOUT" *always* represents a real 
// channel sensing of the MAC over a fixed period of time
// where the request together with the result will be always sent 
// back after the timeout.
//
// The request returned to the MAC layer holds the result
// in the form of a ChannelState object. How the idle and
// rssi info contained in the ChannelState are calculated
// and interpreted depends on the Decider.
//
packet ChannelSenseRequest
{
    int senseMode enum(SenseMode);	// defines the sensing mode (see SenseMode enum)
    simtime_t senseTimeout; 		// Time to listen on the channel
    ChannelState result; 			// The result of the request containing the current state of the channel.
									// See ChannelState for details.
}
