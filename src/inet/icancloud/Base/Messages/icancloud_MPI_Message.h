#ifndef _icancloud_MPI_MESSAGE_H_
#define _icancloud_MPI_MESSAGE_H_

#include "icancloud_MPI_Message_m.h"

namespace inet {

namespace icancloud {

using std::ostringstream;


/**
 * @class icancloud_MPI_Message icancloud_MPI_Message.h "icancloud_MPI_Message.h"
 *   
 * Class that represents a icancloud_MPI_Message.
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class icancloud_MPI_Message: public icancloud_MPI_Message_Base{
	
			
	public:			
		
	   /**
		* Constructor of icancloud_Message
		* @param name Message name
		* @param kind Message kind		* 
		*/
		icancloud_MPI_Message ();
		icancloud_MPI_Message (const char *):icancloud_MPI_Message (){};
		
		
	   /**
		* Constructor of icancloud_Message
		* @param other Message		 
		*/
		icancloud_MPI_Message(const icancloud_MPI_Message& other);
		
		
	   /**
		* = operator
		* @param other Message		 
		*/
		icancloud_MPI_Message& operator=(const icancloud_MPI_Message& other);
		
		
	   /**
		* Method that makes a copy of a icancloud_Message
		*/
		virtual icancloud_MPI_Message *dup() const;

		/**
		* Parse all parameters of current message to string.
		* @return String with the corresponding contents.
		*/
		virtual string contentsToString (bool printContents) const;

						
};

} // namespace icancloud
} // namespace inet

#endif
