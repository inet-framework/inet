#include "icancloud_App_NET_Message.h"

namespace inet {

namespace icancloud {


Register_Class (icancloud_App_NET_Message);

using namespace inet;
using namespace omnetpp;

icancloud_App_NET_Message::~icancloud_App_NET_Message(){
}


icancloud_App_NET_Message::icancloud_App_NET_Message(): icancloud_App_NET_Message_Base(){

    setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_App_NET_Message");
}


icancloud_App_NET_Message::icancloud_App_NET_Message(const icancloud_App_NET_Message& other){
    icancloud_App_NET_Message_Base::operator=(other);
	setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_App_NET_Message");
}


icancloud_App_NET_Message& icancloud_App_NET_Message::operator=(const icancloud_App_NET_Message& other){

	icancloud_App_NET_Message_Base::operator=(other);
	return *this;
}


icancloud_App_NET_Message *icancloud_App_NET_Message::dup() const{

	icancloud_App_NET_Message *newMessage;
	//TCPCommand *controlNew;
	//TCPCommand *controlOld;
	unsigned int i;

		// Create a new message!
		newMessage = new icancloud_App_NET_Message();

		// Base parameters...
		newMessage->setOperation (getOperation());
		newMessage->setIsResponse (getIsResponse());
		newMessage->setRemoteOperation (getRemoteOperation());
		newMessage->setConnectionId (getConnectionId());
		newMessage->setCommId(getCommId());
		newMessage->setSourceId (getSourceId());
		newMessage->setNextModuleIndex (getNextModuleIndex());
		newMessage->setResult (getResult());
        newMessage->setUid(getUid());
        newMessage->setPid(getPid());
		newMessage->setChunkLength (getChunkLength());
		newMessage->setParentRequest (getParentRequest());

		// icancloud_App_NET_Message parameters...
		newMessage->setDestinationIP (getDestinationIP());
		newMessage->setDestinationPort (getDestinationPort());		
		newMessage->setLocalIP (getLocalIP());
		newMessage->setLocalPort (getLocalPort());		
		newMessage->setSize (getSize());		
		newMessage->setData (getData());		

		// User
		newMessage->setVirtual_user(getVirtual_user());

		// IPs
		newMessage->setVirtual_destinationIP(getVirtual_destinationIP());
		newMessage->setVirtual_localIP(getVirtual_localIP());

		// Ports..
		newMessage->setVirtual_destinationPort(getVirtual_destinationPort());
		newMessage->setVirtual_localPort(getVirtual_localPort());

		newMessage->setFsType(getFsType());
		newMessage->setTargetPosition(getTargetPosition());

		// icancloud changing state parameters ..
		newMessage->setChangingState(state);

		for (i=0; i < change_states_index.size(); i++){
			newMessage->add_component_index_To_change_state(change_states_index[i]);
		}

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
		for (i=0; i<trace.size(); i++){
			newMessage->addNodeTrace (trace[i].first, trace[i].second);
		}

	return (newMessage);
}


void icancloud_App_NET_Message::updateLength (){
	
	unsigned long int newSize;
	int operation = getOperation();

	// Set the new size!
	newSize = MSG_INITIAL_LENGTH;
	if (!getIsResponse() ) {
		if ((operation == SM_SEND_DATA_NET) || (operation == SM_ITERATIVE_PRECOPY)){
			newSize = MSG_INITIAL_LENGTH + getSize();
		} 
	}

	setChunkLength (B(newSize));
}


string icancloud_App_NET_Message::contentsToString (bool printContents) const {

	std::ostringstream osStream;

		if (printContents){

			osStream << icancloud_Message::contentsToString(printContents);
	
			osStream  << " - Local IP:" <<  getLocalIP() << endl;
			osStream  << " - Destination IP:" <<  getDestinationIP() << endl;
			osStream  << " - Local Port:" <<  getLocalPort() << endl;
			osStream  << " - Destination Port:" <<  getDestinationPort() << endl;
			osStream  << " - Size:" <<  getSize() << endl;
			osStream  << " - Data:" <<  getData() << endl;
			osStream  << " - Virtual Local IP:" <<  getVirtual_localIP() << endl;
			osStream  << " - Virtual Destination IP:" <<  getVirtual_destinationIP() << endl;
			osStream  << " - Virtual User:" <<  getVirtual_user() << endl;
			osStream  << " - Virtual Local Port:" <<  getVirtual_localPort() << endl;
			osStream  << " - Virtual Destination Port:" <<  getVirtual_destinationPort() << endl;

		}

	return osStream.str();
}


void icancloud_App_NET_Message::parsimPack(cCommBuffer *b) const {

	icancloud_App_NET_Message_Base::parsimPack(b);
}


void icancloud_App_NET_Message::parsimUnpack(cCommBuffer *b){

	icancloud_App_NET_Message_Base::parsimUnpack(b);
}

} // namespace icancloud
} // namespace inet
