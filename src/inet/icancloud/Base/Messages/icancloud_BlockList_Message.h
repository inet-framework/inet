#ifndef _icancloud_BLOCKLIST_MESSAGE_H_
#define _icancloud_BLOCKLIST_MESSAGE_H_

#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message_m.h"

namespace inet {

namespace icancloud {



/**
 * @class icancloud_BlockList_Message icancloud_BlockList_Message.h "Base/Messages/icancloud_BlockList_Message.h"
 *
 * Class that represents a icancloud_BlockList_Message.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class icancloud_BlockList_Message: public icancloud_BlockList_Message_Base{

	public:

	   /**
		* Destructor.
		*/
		virtual ~icancloud_BlockList_Message();

	   /**
		* Constructor of icancloud_Message
		* @param name Message name
		* @param kind Message kind
		*/
		icancloud_BlockList_Message ();
		icancloud_BlockList_Message (const char *) :icancloud_BlockList_Message (){}


	   /**
		* Constructor of icancloud_Message
		* @param other Message
		*/
		icancloud_BlockList_Message (const icancloud_BlockList_Message& other);


	   /**
		* Constructor of icancloud_Message
		* @param sm_io Message
		*/
		icancloud_BlockList_Message (icancloud_App_IO_Message *sm_io);


	   /**
		* = operator
		* @param other Message
		*/
		icancloud_BlockList_Message& operator=(const icancloud_BlockList_Message& other);

	   /**
		* Method that makes a copy of a icancloud_Message
		*/
		virtual icancloud_BlockList_Message *dup() const override;


	   /**
		* Creates a copy of current message returning a icancloud_App_IO_Message object.
		* @return icancloud_App_IO_Message object
		*/

		icancloud_App_IO_Message* transformToApp_IO_Message ();


	   /**
		* Update the message length
		*/
		void updateLength ();

		virtual string contentsToString (bool printContents) const override {return icancloud_App_IO_Message::contentsToString(printContents);}
	   /**
		* Parse all parameters of current message to string.
		* @param withBranches Parse branch information?
		* @param printContents Print contents?
		* @return String with the corresponding contents.
		*/
		virtual string contentsToString (bool withBranches, bool printContents) const;
};

} // namespace icancloud
} // namespace inet

#endif
