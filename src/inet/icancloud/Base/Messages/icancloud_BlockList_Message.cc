#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"

namespace inet {

namespace icancloud {


using namespace inet;
using namespace omnetpp;

Register_Class(icancloud_BlockList_Message);


icancloud_BlockList_Message::~icancloud_BlockList_Message(){
}


icancloud_BlockList_Message::icancloud_BlockList_Message(): icancloud_BlockList_Message_Base(){

	this->setChunkLength(B(MSG_INITIAL_LENGTH));
//	setName ("icancloud_BlockList_Message");
}


icancloud_BlockList_Message::icancloud_BlockList_Message(const icancloud_BlockList_Message& other) {


    icancloud_BlockList_Message_Base::operator=(other);
	setChunkLength(B(MSG_INITIAL_LENGTH));
//	setName ("icancloud_BlockList_Message");
}


icancloud_BlockList_Message::icancloud_BlockList_Message(icancloud_App_IO_Message *sm_io) {
    int i;
    /*TCPCommand *controlNew;
    TCPCommand *controlOld;*/
    setChunkLength(B(MSG_INITIAL_LENGTH));
//    setName("icancloud_BlockList_Message");
    // Base parameters...
    setOperation(sm_io->getOperation());
    setIsResponse(sm_io->getIsResponse());
    setRemoteOperation(sm_io->getRemoteOperation());
    setConnectionId(sm_io->getConnectionId());
    setCommId(sm_io->getCommId());
    setSourceId(sm_io->getSourceId());
    setNextModuleIndex(sm_io->getNextModuleIndex());
    setResult(sm_io->getResult());
    setPid(sm_io->getPid());
    setUid(sm_io->getUid());

    setChunkLength(sm_io->getChunkLength());
    setParentRequest(sm_io->getParentRequest());

    // icancloud_App_IO_Message parameters...
    setFileName(sm_io->getFileName());
    setOffset(sm_io->getOffset());
    setSize(sm_io->getSize());

    // File info...
    getFileForUpdate().setFileName(sm_io->getFileName());
    getFileForUpdate().setFileSize(0);

    // Copy the control info, if exists!
    /*if (sm_io->getControlInfo() != nullptr) {
        controlOld = check_and_cast<TCPCommand *>(sm_io->getControlInfo());
        controlNew = new TCPCommand();
        controlNew = controlOld->dup();
        setControlInfo(controlNew);
    }*/

    // icancloud NFS Message parameters...

    setNfs_destAddress(sm_io->getNfs_destAddress());
    setNfs_destPort(sm_io->getNfs_destPort());
    setNfs_requestSize_KB(sm_io->getNfs_requestSize_KB());
    setNfs_type(sm_io->getNfs_type());
    setNfs_connectionID(sm_io->getNfs_connectionID());

    // Reserve memory to trace!
    setTraceArraySize(sm_io->getTraceArraySize());

    // Copy trace!
    for (i = 0; i < ((int) sm_io->getTraceArraySize()); i++) {
        addNodeTrace(sm_io->getHostName(i), sm_io->getNodeTrace(i));
    }

}


icancloud_BlockList_Message& icancloud_BlockList_Message::operator=(const icancloud_BlockList_Message& other){

	icancloud_BlockList_Message_Base::operator=(other);
	return *this;
}


icancloud_BlockList_Message *icancloud_BlockList_Message::dup() const{

	icancloud_BlockList_Message *newMessage;
//	TCPCommand *controlNew;
//	TCPCommand *controlOld;
	int i;

		// Create a new message!
		newMessage = new icancloud_BlockList_Message();

		// Base parameters...
		newMessage->setOperation (getOperation());
		newMessage->setIsResponse (getIsResponse());
		newMessage->setRemoteOperation (getRemoteOperation());
		newMessage->setConnectionId (getConnectionId());
		newMessage->setSourceId (getSourceId());
		newMessage->setNextModuleIndex (getNextModuleIndex());
		newMessage->setResult (getResult());
		newMessage->setCommId(getCommId());
        newMessage->setUid(getUid());
        newMessage->setPid(getPid());
		newMessage->setChunkLength (getChunkLength());
		newMessage->setParentRequest (getParentRequest());

		// icancloud changing state parameters ..
		newMessage->setChangingState(state);
		for (i=0; i< ((int)change_states_index.size()); i++){
			newMessage->add_component_index_To_change_state(change_states_index[i]);
		}

		// Copy the control info, if exists!
/*		if (getControlInfo() != nullptr){
			controlOld = check_and_cast<TCPCommand *>(getControlInfo());
			controlNew = new TCPCommand();
			controlNew = controlOld->dup();
			newMessage->setControlInfo (controlNew);
		}
*/
		// Reserve memory to trace!
		newMessage->setTraceArraySize (getTraceArraySize());

		// Copy trace!
		for (i=0; i<((int)trace.size()); i++){
			newMessage->addNodeTrace (trace[i].first, trace[i].second);
		}

		// icancloud_App_IO_Message parameters...
		newMessage->setFileName (getFileName());
		newMessage->setOffset (getOffset());
		newMessage->setSize (getSize());
		newMessage->setNfs_destAddress(getNfs_destAddress());
		newMessage->setNfs_destPort(getNfs_destPort());
		newMessage->setNfs_requestSize_KB(getNfs_requestSize_KB());
		newMessage->setNfs_type(getNfs_type());
		newMessage->setNfs_connectionID(getNfs_connectionID());


	return (newMessage);
}



icancloud_App_IO_Message* icancloud_BlockList_Message::transformToApp_IO_Message (){

	icancloud_App_IO_Message *newMessage;
//	TCPCommand *controlNew;
//	TCPCommand *controlOld;
	int i;

		// Create a new message!
		newMessage = new icancloud_App_IO_Message();

		// Base parameters...
		newMessage->setOperation (getOperation());
		newMessage->setIsResponse (getIsResponse());
		newMessage->setRemoteOperation (getRemoteOperation());
		newMessage->setConnectionId (getConnectionId());
		newMessage->setCommId(getCommId());
		newMessage->setSourceId (getSourceId());
		newMessage->setNextModuleIndex (getNextModuleIndex());
		newMessage->setResult (getResult());
		newMessage->setPid(getPid());
		newMessage->setUid(getUid());

		newMessage->setChunkLength (getChunkLength());
		newMessage->setParentRequest (getParentRequest());

		// icancloud_App_IO_Message parameters...
		newMessage->setFileName (getFileName());
		newMessage->setOffset (getOffset());
		newMessage->setSize (getSize());
		newMessage->setNfs_connectionID(getNfs_connectionID());
		newMessage->setNfs_destAddress(getNfs_destAddress());
		newMessage->setNfs_destPort(getNfs_destPort());
		newMessage->setNfs_requestSize_KB(getNfs_requestSize_KB());
		newMessage->setNfs_type(getNfs_type());

		// Copy the control info, if exists!
/*		if (getControlInfo() != nullptr){
			controlOld = check_and_cast<TCPCommand *>(getControlInfo());
			controlNew = new TCPCommand();
			controlNew = controlOld->dup();
			newMessage->setControlInfo (controlNew);
		}
*/
		// Reserve memory to trace!
		newMessage->setTraceArraySize (getTraceArraySize());

		// Copy trace!
		for (i=0; i<((int)trace.size()); i++){
			newMessage->addNodeTrace (trace[i].first, trace[i].second);
		}


	return (newMessage);
}


void icancloud_BlockList_Message::updateLength (){

	unsigned long int newSize;
	std::ostringstream osStream;

	if (!getRemoteOperation()){
		// Request!
		if (!getIsResponse()){

			// READ
			if (((int)getOperation()) == SM_READ_FILE)
				newSize = MSG_INITIAL_LENGTH;

			// WRITE
			else if ( (((int)getOperation()) == SM_WRITE_FILE) ||
			          (((int)getOperation()) == SM_CREATE_FILE) ||
                      (((int)getOperation()) == SM_DELETE_FILE) )
				newSize = MSG_INITIAL_LENGTH + getFile().getFileSize();

			// Unknown operation!
			else{
				osStream << "Error updating icancloud_BlockList_Message length (request). Wrong operation:"
						 << operationToString (getOperation());
				throw cRuntimeError(osStream.str().c_str());
			}
		}

		// Response!
		else{

			// READ
			if (((int)getOperation()) == SM_READ_FILE)
				newSize = MSG_INITIAL_LENGTH + getFile().getFileSize();

			// WRITE
            else if ( (((int)getOperation()) == SM_WRITE_FILE) ||
                      (((int)getOperation()) == SM_CREATE_FILE) ||
                      (((int)getOperation()) == SM_DELETE_FILE) )
				newSize = MSG_INITIAL_LENGTH;

			// Unknown operation!
			else{
				osStream << "Error updating SIMCAM_BlockList_Message length (response). Wrong operation:"
						 << operationToString (getOperation());
				throw cRuntimeError(osStream.str().c_str());
			}
		}
	}
	else
	{
		// Request!
		if (getIsResponse()){

			// READ
			if (((int)getOperation()) == SM_READ_FILE)
				newSize = MSG_INITIAL_LENGTH;

			// WRITE
            else if ( (((int)getOperation()) == SM_WRITE_FILE) ||
                      (((int)getOperation()) == SM_CREATE_FILE) ||
                      (((int)getOperation()) == SM_DELETE_FILE) )
				newSize = MSG_INITIAL_LENGTH + getSize();//+ getFile().getFileSize();

			// Unknown operation!
			else{
				osStream << "Error updating SIMCAM_BlockList_Message length (request). Wrong operation:"
						 << operationToString (getOperation());
				throw cRuntimeError(osStream.str().c_str());
			}
		}

		// Response!
		else{

			// READ
			if (((int)getOperation()) == SM_READ_FILE)
				newSize = MSG_INITIAL_LENGTH + getSize(); //getFile().getFileSize();

			// WRITE
			else if (((int)getOperation()) == SM_WRITE_FILE)
				newSize = MSG_INITIAL_LENGTH;

			// Unknown operation!
			else{
				osStream << "Error updating SIMCAM_BlockList_Message length (response). Wrong operation:"
						 << operationToString (getOperation());
				throw cRuntimeError(osStream.str().c_str());
			}
		}
	}
	// Set the new size!
	this->setChunkLength (B(newSize));
}


string icancloud_BlockList_Message::contentsToString (bool withBranches, bool printContents) const {

	std::ostringstream osStream;
	
		if (printContents){
	
			osStream  << icancloud_App_IO_Message::contentsToString(printContents);
			osStream  << getFile().contentsToString(withBranches, printContents) << endl;
		}

	return osStream.str();
}


} // namespace icancloud
} // namespace inet
