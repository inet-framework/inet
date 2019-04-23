
import inet.icancloud.Base.Messages.icancloud_Message;
import inet.icancloud.Base.Messages.icancloud_App_IO_Message;
cplusplus {{
	#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"			
	#include "inet/icancloud/Base/Messages/icancloud_File.h"
}}



namespace inet::icancloud;

//struct fileBranch;
class icancloud_File { @existingClass; }
//class icancloud_App_IO_Message { @existingClass; }

///
/// icancloud Message for disk requests (branch list)
///
class icancloud_BlockList_Message extends icancloud_App_IO_Message
{
    @customize(true);

    icancloud_File file;		// Branch list
}

