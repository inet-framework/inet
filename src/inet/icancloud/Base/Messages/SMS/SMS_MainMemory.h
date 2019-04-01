#ifndef __SMS_MAIN_MEMORY_
#define __SMS_MAIN_MEMORY_

#include <omnetpp.h>
#include <list>
#include "inet/icancloud/Base/Messages/SMS/SplittingMessageSystem.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_MEM_Message.h"

namespace inet {

namespace icancloud {



/**
 * @class SMS_MainMemory SMS_MainMemory.h "Base/Messages/SMS/SMS_MainMemory.h"
 *
 * Class that implements a SplittingMessageSystem abstract class.
 *
 * Specific features of this class:
 *
 * - Performs a linear searching and linear deleting.
 * - Inserts requests following a FIFO algorithm.
 * - Splitting method: Genetares a subrequest per memory block size.
 * - Supported message type: icancloud_App_IO_Message
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class SMS_MainMemory : public SplittingMessageSystem {

	protected:

		/** Memory size (in bytes)*/
		unsigned int memorySize;

		/** ReadAheadBlocks blocks */
	    unsigned int readAheadBlocks;

	    /** Memory block size (in bytes) */
		unsigned int memoryBlockSize;

		/** List of subRequests */
		std::list <inet::Packet *> subRequests;


	public:

	   /**
		* Constructor.
		*
		* @param newCacheSize newCacheSize Cache size (in bytes)
		* @param newReadAheadBlocks Number of blocks to pre-fetching (read ahead)
		* @param newCacheBlockSize Cache block size (in bytes)
		*/
		SMS_MainMemory (unsigned int newCacheSize, unsigned int newReadAheadBlocks, unsigned int newCacheBlockSize);

	   /**
 		* Given a request size, this function split a request in <b>requestSizeKB</b> KB subRequests.
 		* @param msg Request message.
 		*/
		void splitRequest (omnetpp::cMessage*msg) override;
		
	   /**
		* Calculates if required block from the original request have arrived.
		* This blocks not take into account the read-ahead blocks.
		* @param request Original request.
		* @param extraBlocks Number of read-ahead blocks.
		* @return true if original request blocks have been arrived, or false in another case..
		*/
		bool arrivesRequiredBlocks(inet::Packet* request, unsigned int extraBlocks);

	   /**
		* Gets the first subRequest. This method does not remove the subRequest from list.
		* @return First subRequest if list is not empty or nullptr if list is empty.
		*/
		inet::Packet * getFirstSubRequest();

	   /**
		* Pops the first subRequest. This method removes the subRequest from list.
		* @return First subRequest if list is not empty or nullptr if list is empty.
		*/
		Packet* popSubRequest();

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
