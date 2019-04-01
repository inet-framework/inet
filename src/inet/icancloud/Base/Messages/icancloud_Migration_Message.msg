import inet.icancloud.Base.Messages.icancloud_Message;
import inet.icancloud.Base.Messages.icancloud_App_NET_Message;

cplusplus {{
	#include <vector>
    #include "inet/icancloud/Base/Messages/icancloud_App_NET_Message.h"
}}

namespace inet::icancloud;

//class  VmPtr { @existingClass; }
//class icancloud_App_NET_Message { @existingClass; }
//
// icancloud Message for migration requests
//
class icancloud_Migration_Message extends icancloud_App_NET_Message
{
    @customize(true);   
    int memorySizeKB = 0;
    int diskSizeKB = 0;
    string virtualIP = "";
    
	bool remoteStorage = false;
	int standardRequests = 0;
	int deleteRequests = 0;
	int remoteStorageUsedKB = 0;
	
}

