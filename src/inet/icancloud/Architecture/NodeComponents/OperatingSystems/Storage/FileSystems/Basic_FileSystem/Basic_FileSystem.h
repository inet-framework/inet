#ifndef __BASIC_FILE_SYSTEM_H_
#define __BASIC_FILE_SYSTEM_H_

#include <omnetpp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/Messages/icancloud_File.h"
#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"

namespace inet {

namespace icancloud {


/**
 * @class Basic_FileSystem Basic_FileSystem.h "Basic_FileSystem.h"
 *
 * This module translates an I/O file request expressed as (fileName, offset, requestSize) to a list of disk blocks.
 * This module only is useful with one tenant at system. It is not able to distinguish between more ones.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 *
 * @author updated to iCanCloud by Gabriel González Castañé
 * @date 2012-05-17
 *
 */
class Basic_FileSystem: public icancloud_Base{

	protected:		
		
		/** Gate ID. Input gate from VM. */
		cGate* fromVMGate;

		/** Gate ID. Input gate from IOR. */
		cGate* fromIORGate;

		/** Gate ID. Output gate to VM. */
		cGate* toVMGate;

		/** Gate ID. Output gate to IOR. */
		cGate* toIORGate;

	    /** Pending message */
	    cMessage *pendingMessage = nullptr;

		/** File list*/
		vector <icancloud_File> fileList;

		/** Open Time */
		static const const_simtime_t OPEN_TIME;

		/** Close Time */
		static const const_simtime_t CLOSE_TIME;
		
		/** Create Time */
		static const const_simtime_t CREATE_TIME;



	   /**
		* Destructor. 
		*/
		~Basic_FileSystem();

	   /**
	 	* Module initialization.
	 	*/
	    virtual void initialize(int stage) override;
	    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    

	   /**
	 	* Module ending.
	 	*/
	    virtual void finish() override;


	private:

	   /**
		* Get the outGate to the module that sent <b>msg</b>
		* @param msg Arrived message.
		* @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
		*/
		cGate* getOutGate (cMessage *msg) override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		void processSelfMessage (cMessage *msg) override;;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		void processRequestMessage (Packet *) override;

	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;

	   /**
		* Calculates the involved blocks (list of branches) of an I/O request and sends the request to Block Server.
		*
		* @param sm StandardMessage that contains the I/O request operation.
		*/
		void processIORequest (Packet *);

	   /**
		* Inserts a new file inside the File System's table.
		*
		* @param fileSize File's size.
		* @param fileName File's name.
		* @return icancloud_OK if the file was been inserted correctly, or icancloud_ERROR if something was wrong.
		*/
		int insertNewFile (unsigned int fileSize, string fileName);

	   /**
		* Search a file in the file system.
		*
		* @param fileName File name.
		* @return Iterator to the corresponding entry of <b>fileName</b>
		*/
		vector <icancloud_File>::iterator searchFile (string fileName);

	   /**
		* Romoves a file from the File System's table.
		*
		* @param fileName File's name.
		*/
		void deleteFile (string fileName);


	   /**
		* Translate an user I/O request into a branch list.
		* If somethings goes wrong (file not exist, request out of file size,... ) the returned
		* string will be empty.
		*
		* @param sm_bl Block list Message.
		*/
		void translateIORequets (Packet *);

	   /**
		* Parse all files information to string. Debug purpose only!
		* @return All files stored on FS in string format.
		*/
		string FSFilesToString ();
};

} // namespace icancloud
} // namespace inet

#endif
