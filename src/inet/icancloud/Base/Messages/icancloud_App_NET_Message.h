#ifndef _icancloud_APP_NET_MESSAGE_H_
#define _icancloud_APP_NET_MESSAGE_H_

#include "icancloud_App_NET_Message_m.h"

namespace inet {

namespace icancloud {


/**
 * @class icancloud_App_NET_Message icancloud_App_NET_Message.h "icancloud_App_NET_Message.h"
 *
 * Class that represents a icancloud_App_NET_Message.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class icancloud_App_NET_Message: public icancloud_App_NET_Message_Base{


	public:

	   /**
		* Destructor.
		*/
		virtual ~icancloud_App_NET_Message();

	   /**
		* Constructor of icancloud_Message
		* @param name Message name
		* @param kind Message kind
		*/
		icancloud_App_NET_Message ();
		icancloud_App_NET_Message (const char *) : icancloud_App_NET_Message (){}


	   /**
		* Constructor of icancloud_Message
		* @param other Message
		*/
		icancloud_App_NET_Message(const icancloud_App_NET_Message& other);


	   /**
		* = operator
		* @param other Message
		*/
		icancloud_App_NET_Message& operator=(const icancloud_App_NET_Message& other);


	   /**
		* Method that makes a copy of a icancloud_Message
		*/
		virtual icancloud_App_NET_Message *dup() const override;


	   /**
		* Update the message length
		*/
		void updateLength ();
   

	   /**
		* Parse all parameters of current message to string.
		* @return String with the corresponding contents.
		*/
		virtual string contentsToString (bool printContents) const override;


	   /**
		* Serializes a icancloud_Message.
		* @param b Communication buffer
		*/
		virtual void parsimPack(omnetpp::cCommBuffer *b) const override;


	   /**
		* Deserializes a icancloud_Message.
		* @param b Communication buffer
		*/
		virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

} // namespace icancloud
} // namespace inet

#endif
