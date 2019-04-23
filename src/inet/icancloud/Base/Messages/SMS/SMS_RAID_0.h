#ifndef __SMS_RAID_0_
#define __SMS_RAID_0_

#include <omnetpp.h>
#include "inet/icancloud/Base/Messages/SMS/SplittingMessageSystem.h"
#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"

namespace inet {

namespace icancloud {



/**
 * @class SMS_RAID_0 SMS_RAID_0.h "Base/Messages/SMS_RAID_0.h"
 *
 * Class that implements a SplittingMessageSystem abstract class.
 *
 * Specific features of this class:
 *
 * - Performs a linear searching and linear deleting.
 * - Inserts requests following a FIFO algorithm.
 * - Splitting method: Splits an I/O request into strideSize (bytes) subRequests.
 * - Supported message type: icancloud_BlockList_Message.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class SMS_RAID_0 : public SplittingMessageSystem {

	protected:

		/** Stride size (in bytes) */
		unsigned int strideSize;

		/** Number of blocks (of BYTES_PER_SECTOR bytes) per request. */
		unsigned int blocksPerRequest;

		/** Number of disks connected to Volume Manager */
		unsigned int numDisks;

		/** Show branch detailed info? */
		static const bool SHOW_BRANCHES;



	public:

	   /**
		* Constructor.
		*
		* @param newStrideSize Size (in bytes) of each file slide.
		* @param numBlockServers Number of blockServers connected to Volume Manager
		*/
		SMS_RAID_0 (unsigned int newStrideSize, unsigned int numBlockServers);


	   /**
 		* Given a request size, this function split a request in <b>stribeSize</b> KB subRequests.
 		* @param msg Request message.
 		*/
		void splitRequest (omnetpp::cMessage *msg) override;


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

#endif /*__SMS_RAID_0_*/
