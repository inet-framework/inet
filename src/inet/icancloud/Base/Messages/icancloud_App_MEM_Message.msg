import inet.icancloud.Base.Messages.icancloud_Message;
cplusplus {{
#include "inet/icancloud/Base/Messages/icancloud_Message.h"
}}

namespace inet::icancloud;
//class icancloud_Message { @existingClass; }
///
/// icancloud Message for MEM requests
///
class icancloud_App_MEM_Message extends icancloud_Message
{
    @customize(true);

    unsigned int memSize = 0;		// Amount of memory
    unsigned int region = 0;		// Region of memory
}
