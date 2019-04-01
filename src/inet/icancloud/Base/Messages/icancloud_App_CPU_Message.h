#ifndef _icancloud_APP_CPU_MESSAGE_H_
#define _icancloud_APP_CPU_MESSAGE_H_

#include "icancloud_App_CPU_Message_m.h"

namespace inet {

namespace icancloud {



/**
 * @class icancloud_App_CPU_Message icancloud_App_CPU_Message.h "icancloud_App_CPU_Message.h"
 *
 * Class that represents a icancloud_App_CPU_Message.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class icancloud_App_CPU_Message: public icancloud_App_CPU_Message_Base{


	public:

	   /**
		* Destructor.
		*/
		virtual ~icancloud_App_CPU_Message();

	   /**
		* Constructor of icancloud_Message
		* @param name Message name
		* @param kind Message kind
		*/
		icancloud_App_CPU_Message ();
		icancloud_App_CPU_Message (const char *) : icancloud_App_CPU_Message (){}


	   /**
		* Constructor of icancloud_Message
		* @param other Message
		*/
		icancloud_App_CPU_Message(const icancloud_App_CPU_Message& other);


	   /**
		* = operator
		* @param other Message
		*/
		icancloud_App_CPU_Message& operator=(const icancloud_App_CPU_Message& other);


	   /**
		* Method that makes a copy of a icancloud_Message
		*/
		virtual icancloud_App_CPU_Message *dup() const override;


	   /**
		* Update the message length
		*/
		void updateLength ();
		
		
	   /**
		* Update the MIs to be exucuted
		* @param numberMIs Number of instructions (measured in MI) to be executed
		*/
		void executeMIs (unsigned int numberMIs);
   

	   /**
		* Update the amount of time to be exucuted
		* @param executedTime Amount of time for executing current CB
		*/
		void executeTime (omnetpp::simtime_t executedTime);


	   /**
		* Parse all parameters of current message to string.
		* @param printContents Print message contents.
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
