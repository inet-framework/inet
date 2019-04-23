#include "inet/icancloud/Base/Messages/icancloud_App_MEM_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"

namespace inet {

namespace icancloud {


using namespace inet;
using namespace omnetpp;

Register_Class (icancloud_App_MEM_Message);


icancloud_App_MEM_Message::~icancloud_App_MEM_Message(){
}


icancloud_App_MEM_Message::icancloud_App_MEM_Message(): icancloud_App_MEM_Message_Base(){

    setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_App_MEM_Message");
}


icancloud_App_MEM_Message::icancloud_App_MEM_Message(const icancloud_App_MEM_Message& other){

	operator=(other);
	setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_App_MEM_Message");
}


icancloud_App_MEM_Message& icancloud_App_MEM_Message::operator=(const icancloud_App_MEM_Message& other){

	icancloud_App_MEM_Message_Base::operator=(other);

	return *this;
}


icancloud_App_MEM_Message *icancloud_App_MEM_Message::dup() const{

	icancloud_App_MEM_Message *newMessage;
	//TCPCommand *controlNew;
	//TCPCommand *controlOld;
	unsigned int i;

		// Create a new message!
		newMessage = new icancloud_App_MEM_Message();

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
		newMessage->setChunkLength(getChunkLength());
		newMessage->setParentRequest (getParentRequest());

		// icancloud_App_MEM_Message parameters...
		newMessage->setMemSize (getMemSize());
		newMessage->setRegion (getRegion());
		
		// icancloud changing state parameters ..
		newMessage->setChangingState(state);
		for (i=0; i< change_states_index.size(); i++){
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


icancloud_Message *icancloud_App_MEM_Message::dupGeneric() const{

	icancloud_App_IO_Message *newMessage;
	unsigned int i;

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
		newMessage->setUid(getUid());
		newMessage->setPid(getPid());
		newMessage->setChunkLength(getChunkLength());
		newMessage->setParentRequest (getParentRequest());

		// Copy the control info, if exists!
		//if (getControlInfo() != nullptr){
			//newMessage->setControlInfo (getControlInfo());
		//}

		// Reserve memory to trace!
		newMessage->setTraceArraySize (getTraceArraySize());

		// Copy trace!
		for (i=0; i<trace.size(); i++){
			newMessage->addNodeTrace (trace[i].first, trace[i].second);
		}

	return (newMessage);
}


void icancloud_App_MEM_Message::updateLength (){
	
		// Set the new size!
    this->setChunkLength(B(MSG_INITIAL_LENGTH));
}


string icancloud_App_MEM_Message::contentsToString (bool printContents) const {

	std::ostringstream osStream;

		if (printContents){

			osStream << icancloud_Message::contentsToString(printContents);
			osStream  << " - Requested mem size:" <<  getMemSize() << endl;
			osStream  << " - Memory region:" <<  regionToString(getRegion()) << endl;
		}

	return osStream.str();
}


string icancloud_App_MEM_Message::regionToString (int region) const {
	
	string result;

		if (region == SM_MEMORY_REGION_CODE)
			result = "SM_MEMORY_REGION_CODE";

		else if (region == SM_MEMORY_REGION_LOCAL_VAR)
			result = "SM_MEMORY_REGION_LOCAL_VAR";

		else if (region == SM_MEMORY_REGION_GLOBAL_VAR)		
			result = "SM_MEMORY_REGION_GLOBAL_VAR";
						
		else if (region == SM_MEMORY_REGION_DYNAMIC_VAR)
			result = "SM_MEMORY_REGION_DYNAMIC_VAR";
						
		else 
			result = "REGION not found!";
			
	return result;
}


void icancloud_App_MEM_Message::parsimPack(cCommBuffer *b) const {

	icancloud_App_MEM_Message_Base::parsimPack(b);
}


void icancloud_App_MEM_Message::parsimUnpack(cCommBuffer *b){

	icancloud_App_MEM_Message_Base::parsimUnpack(b);
}




} // namespace icancloud
} // namespace inet
