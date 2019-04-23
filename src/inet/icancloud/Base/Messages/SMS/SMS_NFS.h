#ifndef __SMS_NFS_
#define __SMS_NFS_

#include <omnetpp.h>
#include "SplittingMessageSystem.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"

namespace inet {

namespace icancloud {



/**
 * @class SMS_NFS SMS_NFS.h "SMS_NFS.h"
 *
 * Class that implements a SplittingMessageSystem abstract class.
 *
 * Specific features of this class:
 *
 * - Performs a linear searching and linear deleting.
 * - Inserts requests following a FIFO algorithm.
 * - Splitting method: Split an I/O request into NFSRequests subRequests.
 *   Each new generated NFSRequest will contain a portion (in KB) of the original I/O request.
 *   The size of the generated NFSRequest is configurable.
 * - Supported message type: icancloud_App_IO_Message
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class SMS_NFS : public SplittingMessageSystem {

	protected:

		/** subRequest size (in bytes) */
		int requestSizeNFS;

	public:

	   /**
		* Constructor.
		*
		* @param requestSize_KB Size (in KB) of the new generated requests to be sent to NFS server.
		*/
		SMS_NFS (int requestSize_KB);


	   /**
 		* Given a request size, this function split a request in <b>requestSizeKB</b> KB subRequests.
 		* @param msg Request message.
 		*/
		void splitRequest (cMessage *msg) override;


	   /**
		* Parses a request (with the corresponding sub_request) to string.
		* IMPORTANT: This function must be called next to <b>splitRequest</b>.
		* If not, maybe some sub_requests not been linked to original request.
		* This function is for debugging purpose only.
		*
		* @param index Position ov the requested request.
		* @return A string with the corresponding request info.
		*/
		string requestToStringByIndex (unsigned int index) override;
		
		
	   /**
 		* Removes all requests
 		*/
		void clear () override;
};

} // namespace icancloud
} // namespace inet

#endif /*__SMS_BLOCKSIZE_*/
