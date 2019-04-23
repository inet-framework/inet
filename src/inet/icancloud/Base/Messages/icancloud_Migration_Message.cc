#include "inet/icancloud/Base/Messages/icancloud_Migration_Message.h"

namespace inet {

namespace icancloud {


using namespace inet;

Register_Class(icancloud_Migration_Message);

icancloud_Migration_Message::~icancloud_Migration_Message(){
}

icancloud_Migration_Message::icancloud_Migration_Message(){

    setChunkLength(B(MSG_INITIAL_LENGTH));
	// setName ("icancloud_Migration_Message");
}


icancloud_Migration_Message::icancloud_Migration_Message(const icancloud_Migration_Message& other){

	operator=(other);
	setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_Migration_Message");
}


icancloud_Migration_Message& icancloud_Migration_Message::operator=(const icancloud_Migration_Message& other){

    icancloud_Migration_Message_Base::operator=(other);

	return *this;
}


icancloud_Migration_Message *icancloud_Migration_Message::dup() const{

	icancloud_Migration_Message *newMessage;
	//TCPCommand *controlNew;
	//TCPCommand *controlOld;

	int i;

		// Create a new message!
		newMessage = new icancloud_Migration_Message();

		// Base parameters...
		newMessage->setOperation (getOperation());
		newMessage->setIsResponse (getIsResponse());
		newMessage->setRemoteOperation (getRemoteOperation());
		newMessage->setConnectionId (getConnectionId());
		newMessage->setCommId(getCommId());
		newMessage->setSourceId (getSourceId());
		newMessage->setNextModuleIndex (getNextModuleIndex());
		newMessage->setResult (getResult());
		newMessage->setChunkLength (getChunkLength());
		newMessage->setParentRequest (getParentRequest());
        newMessage->setUid(getUid());
        newMessage->setPid(getPid());
		// icancloud_App_NET_Message parameters...
		newMessage->setDestinationIP (getDestinationIP());
		newMessage->setDestinationPort (getDestinationPort());		
		newMessage->setLocalIP (getLocalIP());
		newMessage->setLocalPort (getLocalPort());		
		newMessage->setSize (getSize());		
		newMessage->setData (getData());

		// icancloud_Migration_Message parameters...
		newMessage->setMemorySizeKB(getMemorySizeKB());
		newMessage->setDiskSizeKB(getDiskSizeKB());
		newMessage->setVirtualIP(getVirtualIP());
		newMessage->setRemoteStorage(getRemoteStorage());
		newMessage->setStandardRequests(getStandardRequests());
		newMessage->setDeleteRequests(getDeleteRequests());
		newMessage->setRemoteStorageUsedKB(getRemoteStorageUsedKB());

		
		// Copy the control info, if exists!
		/*if (getControlInfo() != nullptr){
			controlOld = check_and_cast<TCPCommand *>(getControlInfo());
			controlNew = new TCPCommand();
			controlNew = controlOld->dup();
			newMessage->setControlInfo (controlNew);
		}*/
		
		// Reserve memory to trace!
		newMessage->setTraceArraySize (getTraceArraySize());

		// Copy trace!
		for (i=0; i< ((int)trace.size()); i++){
			newMessage->addNodeTrace (trace[i].first, trace[i].second);
		}

	return (newMessage);


}


} // namespace icancloud
} // namespace inet
