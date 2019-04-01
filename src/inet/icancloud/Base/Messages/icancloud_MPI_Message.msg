import inet.icancloud.Base.Messages.icancloud_Message;
import inet.icancloud.Base.Messages.icancloud_App_NET_Message;
cplusplus {{
    #include "inet/icancloud/Base/Messages/icancloud_App_NET_Message.h"
}}

namespace inet::icancloud;
///
/// icancloud Message for MPI calls
///
//class icancloud_App_NET_Message { @existingClass; }
class icancloud_MPI_Message extends icancloud_App_NET_Message
{
    @customize(true);

    unsigned int sourceRank = 0;	// Rank of the source process
    unsigned int destRank = 0;		// Rank of the destination process
    string fileName = "";			// File name
    int offset = 0;					// Offset				
}
