#ifndef __SPLITTING_MESSAGE_SYSTEM__
#define __SPLITTING_MESSAGE_SYSTEM__

#include "inet/icancloud/Base/Messages/icancloud_Message.h"
#include "inet/icancloud/Base/Messages/SMS/icancloud_Request.h"
#include "inet/icancloud/Base/include/icancloud_types.h"

namespace inet {

namespace icancloud {



/**
 * @class SplittingMessageSystem SplittingMessageSystem.h "SplittingMessageSystem.h"
 *
 * Abstract class that manages Requests.
 *
 * Basically this class splits Requests into SubRequests and sends
 * those SubRequests to the corresponding module. Next, awaits to all SubRequests arrives.
 * When all SubRequests arrives, then the original Requests will be sent back.
 *
 * All derived classes must implement the pure virtual method splitRequest.
 * If needed, a class must re-implement a method.
 *
 * Specific features of this class:
 *
 * - Performs a linear searching and linear deleting.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */

using namespace omnetpp;
using namespace inet;

class SplittingMessageSystem{


	protected:

		/** Requests vector */
		vector <icancloud_Request*> requestVector;
		
		/** Verbose mode? */
	   	static const bool verboseMode;
		

	public:		

	   /**
		* Search for the icancloud_Request that contains the <b>request</b> message
		* @param request Request message to search
		* @return Index to the position of the vector that contains the searched request or NOT_FOUND if <b>request</b> does not exist.
		*/
		virtual int searchRequest (Packet* request);


	   /**
		* Gets the number of subRequest of the icancloud_Request that contains the <b>request</b> message
		* @param request Request message.
		* @return Number of subrequests
		*/
		virtual int getNumberOfSubRequest (Packet* request);

	   /**
		* Check if all subRequests belonging to <b>request</b> have arrived.
		* @param request Request
		* @return True if all subRequest have arrived or false in another case.
		*/
		virtual bool arrivesAllSubRequest (Packet* request);

	   /**
		* Adds a new icancloud_Request that contains the <b>request</b> message.
		* @param request Request to be added
		* @param numSubRequests Number of subRequests
		*/
		virtual void addRequest (Packet* request, unsigned int numSubRequests);

	   /**
		* Removes the icancloud_Request that contains the <b>request</b> message
		* @param request Request to be removed
		*/
		virtual void removeRequest (Packet* request);

	   /**
		* Sets a subRequest to the corresponding request.
		* @param request Request message.
		* @param subRequest SubRequest message.
		* @param subReqIndex SubRequest index.
		*/
		virtual void setSubRequest (Packet* request, Packet* subRequest, unsigned int subReqIndex);

	   /**
		* Get the <b>subRequestIndex</b> subRequest from parent request <b>parentRequest</b>
		* @param parentRequest Parent request
		* @param subRequestIndex SubRequest index
		* @return Corresponding subRequest or nullptr if parent request not found.
		*/
		//icancloud_Message* popSubRequest (Packet* parentRequest, int subRequestIndex);
		Packet* popSubRequest (Packet* parentRequest, int subRequestIndex);


		/**
		* Get the next subRequest from parent request <b>parentRequest</b>
		* @param parentRequest Parent request
		* @return Corresponding subRequest or nullptr if parent request not found.
		*/
		//icancloud_Message* popNextSubRequest (Packet* parentRequest);
		Packet* popNextSubRequest (Packet* parentRequest);


	   /**
		* Link the arrived subRequest and update the number of arrived subRequest
		* belonging to parentRequest. Parent Request must exists, in other case,
		* an exception will be launched!
		* @param subRequest Arrived subRequest.
		* @param parentRequest Parent request.
		*/
		void arrivesSubRequest (Packet* subRequest, Packet* parentRequest);


	   /**
		* Get the total number of requests.
		* @return Number of requests.
		*/
		int getNumberOfRequests ();


	   /**
		* Parses a request (with the corresponding sub_request) to string.
		* IMPORTANT: This function must be called next to <b>splitRequest</b>.
		* If not, maybe some sub_requests not been linked to original request.
		* This function is for debugging purpose only.
		*
		* @param request Original request.
		* @param printContents Prints request contents
		* @return A string with the corresponding request info.
		*/
		string requestToString (Packet* request, bool printContents);


	   /**
 		* Pure virtual function that splits a request in subRequests.
 		* Must be implemented on derived classes.
 		* @param msg Request message.
 		*/
		virtual void splitRequest (omnetpp::cMessage *msg) = 0;


	   /**
		* Parses a request (with the corresponding sub_request) to string.
		* IMPORTANT: This function must be called next to <b>splitRequest</b>.
		* If not, maybe some sub_requests not been linked to original request.
		* This function is for debugging purpose only.
		*
		* @param index Position ov the requested request.
		* @return A string with the corresponding request info.
		*/
		virtual string requestToStringByIndex (unsigned int index) = 0;
		
	   /**
 		* Removes all requests
 		*/
		virtual void clear () = 0;

		virtual ~SplittingMessageSystem();
};


} // namespace icancloud
} // namespace inet

#endif /*__SPLITTINGMESSAGESYSTEM_*/
