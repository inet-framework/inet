import inet.icancloud.Base.Messages.icancloud_Message;
cplusplus {{
#include "inet/icancloud/Base/Messages/icancloud_Message.h"
}}

namespace inet::icancloud;
//class icancloud_Message { @existingClass; }

///
/// icancloud Message for CPU requests
///
class icancloud_App_CPU_Message extends icancloud_Message
{
    @customize(true);
    simtime_t cpuTime = 0.0;				// CPU execution time
    unsigned long totalMIs = 0;			// Number of instructions to be executed.
    unsigned long remainingMIs = 0;		// Remaining instructions to be executed.
    int quantum = 0;					// Quantum used in Round-Robin algorithm (measured in ticks)
    int cpuPriority = 0;					// Priority 
}


