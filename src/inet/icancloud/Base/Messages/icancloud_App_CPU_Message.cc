#include "icancloud_App_CPU_Message.h"

namespace inet {

namespace icancloud {


using namespace inet;
using namespace omnetpp;

Register_Class (icancloud_App_CPU_Message);


icancloud_App_CPU_Message::~icancloud_App_CPU_Message(){
}


icancloud_App_CPU_Message::icancloud_App_CPU_Message(): icancloud_App_CPU_Message_Base(){

	this->setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_App_CPU_Message");
}


icancloud_App_CPU_Message::icancloud_App_CPU_Message(const icancloud_App_CPU_Message& other){

    icancloud_App_CPU_Message_Base::operator=(other);
	this->setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_App_CPU_Message");
}


icancloud_App_CPU_Message& icancloud_App_CPU_Message::operator=(const icancloud_App_CPU_Message& other){

	icancloud_App_CPU_Message_Base::operator=(other);

	return *this;
}


icancloud_App_CPU_Message *icancloud_App_CPU_Message::dup() const{

	icancloud_App_CPU_Message *newMessage;
	//TCPCommand *controlNew;
	//TCPCommand *controlOld;
	unsigned int i;

		// Create a new message!
		newMessage = new icancloud_App_CPU_Message();

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

		// icancloud_App_CPU_Message parameters...
		newMessage->setCpuTime (getCpuTime());
		newMessage->setTotalMIs (getTotalMIs());
		newMessage->setRemainingMIs (getRemainingMIs());
		newMessage->setCpuPriority (getCpuPriority());
		newMessage->setQuantum (getQuantum());
		
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


void icancloud_App_CPU_Message::updateLength (){
	
		// Set the new size!
    this->setChunkLength(B(MSG_INITIAL_LENGTH));
	//	setByteLength (MSG_INITIAL_LENGTH);
}


void icancloud_App_CPU_Message::executeMIs (unsigned int numberMIs){
	
    int test;

	test = (getRemainingMIs() - numberMIs);

	if (test > 0)
		setRemainingMIs (test);
	else 
		setRemainingMIs (0);
}

void icancloud_App_CPU_Message::executeTime (simtime_t executedTime){
	
	simtime_t remainingTime;
	
	remainingTime = getCpuTime();	
	
	if ((remainingTime - executedTime) > 0.0)
		setCpuTime (remainingTime - executedTime);
	else 
		setCpuTime (0);
}



string icancloud_App_CPU_Message::contentsToString (bool printContents) const {

	std::ostringstream osStream;

		if (printContents){

			osStream << icancloud_Message::contentsToString(printContents);
			osStream  << " - cpuTime:" <<  getCpuTime() << endl;
			osStream  << " - totalMIs:" <<  getTotalMIs() << endl;
			osStream  << " - remainingMIs:" <<  getRemainingMIs() << endl;
			osStream  << " - CPU priority:" <<  getCpuPriority() << endl;
			osStream  << " - quantum:" <<  getQuantum() << endl;			
		}

	return osStream.str();
}


void icancloud_App_CPU_Message::parsimPack(cCommBuffer *b) const{

	icancloud_App_CPU_Message_Base::parsimPack(b);
}


void icancloud_App_CPU_Message::parsimUnpack(cCommBuffer *b){

	icancloud_App_CPU_Message_Base::parsimUnpack(b);
}

} // namespace icancloud
} // namespace inet
