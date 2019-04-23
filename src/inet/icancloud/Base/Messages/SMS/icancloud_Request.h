#ifndef __icancloud_REQUEST__
#define __icancloud_REQUEST__

#include "inet/icancloud/Base/Messages/icancloud_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"

namespace inet {

namespace icancloud {


using namespace inet;
/**
 * Class that represents a Request.
 */
class icancloud_Request{

	Packet *parentRequest;			/**< Request request */
	vector <Packet *> subRequests;		/**< Vector with pointers to subRequests */
	unsigned int arrivedSubRequests;	/**< Number of arrived subRequests */

	
	public:
	   
	   /**
		* Constructor.
		*/
		icancloud_Request ();
		
	   /**
		* Constructor.
		* @param newParent Request message
		*/
		icancloud_Request (Packet *newParent);
	
		/**
		* Constructor.
		* @param newParent Request message
		* @param numSubReq Number of subRequest
		*/
		icancloud_Request (Packet *newParent, unsigned int numSubReq);
		
	   /**
		* Destructor.
		*/
		~icancloud_Request ();	
	
		
		Packet* getParentRequest ();
		void setParentRequest (Packet* newParent);
		unsigned int getNumSubRequest ();
		unsigned int getNumArrivedSubRequest () const;
		void setSubRequest (Packet* subRequest, unsigned int index);
		void addSubRequest (Packet* subRequest);
		Packet* getSubRequest (unsigned int index);
		Packet* popSubRequest (unsigned int index);
		Packet* popNextSubRequest ();
		bool arrivesAllSubRequest ();
		void arrivesSubRequest (Packet* subRequest, unsigned int index);
		void clearSubRequests();
		void clear ();
};


} // namespace icancloud
} // namespace inet

#endif /*__icancloud_REQUEST__*/
