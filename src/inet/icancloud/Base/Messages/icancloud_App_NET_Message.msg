import inet.icancloud.Base.Messages.icancloud_Message;
cplusplus {{
#include "inet/icancloud/Base/Messages/icancloud_Message.h"
}}

namespace inet::icancloud;
//class icancloud_Message { @existingClass; }
///
/// icancloud Message for NET requests
///
class icancloud_App_NET_Message extends icancloud_Message
{
    @customize(true);

    string destinationIP = "";		// Destination IP
    string localIP = "";			// Local IP
    int destinationPort = 0;		// Destination port
    int localPort = 0;				// Local port
    string connectionType = "";		// Connection type
    unsigned int size = 0;          // Request size
    string data = "";          		// Data sended
    string virtual_destinationIP = "";
    string virtual_localIP = "";
    int virtual_user = -1;
    int virtual_destinationPort = 0;		// Virtual Destination port
    int virtual_localPort = 0;				// Virtual Local port
	string fsType = "";						// Type of remote file system
	int targetPosition = 1;					// Target Position of remote file system
}
