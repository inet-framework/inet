#ifndef _icancloud_APP_MEM_MESSAGE_H_
#define _icancloud_APP_MEM_MESSAGE_H_

#include "icancloud_App_MEM_Message_m.h"

namespace inet {

namespace icancloud {



/**
 * @class icancloud_App_MEM_Message icancloud_App_MEM_Message.h "icancloud_App_MEM_Message.h"
 *
 * Class that represents a icancloud_App_MEM_Message.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class icancloud_App_MEM_Message: public icancloud_App_MEM_Message_Base{


	public:

    virtual const inet::Ptr<icancloud_Message> dupSharedGeneric() const { return inet::Ptr<icancloud_Message>(static_cast<icancloud_Message *>(dupGeneric())); };


	   /**
		* Destructor.
		*/
		virtual ~icancloud_App_MEM_Message();

	   /**
		* Constructor of icancloud_Message
		* @param name Message name
		* @param kind Message kind
		*/
		icancloud_App_MEM_Message ();
		icancloud_App_MEM_Message (const char *) : icancloud_App_MEM_Message (){}


	   /**
		* Constructor of icancloud_Message
		* @param other Message
		*/
		icancloud_App_MEM_Message(const icancloud_App_MEM_Message& other);


	   /**
		* = operator
		* @param other Message
		*/
		icancloud_App_MEM_Message& operator=(const icancloud_App_MEM_Message& other);


	   /**
		* Method that makes a copy of a icancloud_Message
		*/
		virtual icancloud_App_MEM_Message *dup() const override;


		virtual icancloud_Message *dupGeneric() const;


	   /**
		* Update the message length
		*/
		void updateLength ();
   

	   /**
		* Parse all parameters of current message to string.
		* @param printContents Print message contents.
		* @return String with the corresponding contents.
		*/
		virtual string contentsToString (bool printContents) const override;


	   /**
		* 
		* Parse a memory region to string format
		* 
		* @param region Memory region
		* @return Memory region in string format
		*  
		*/
		virtual string regionToString (int region) const;


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
