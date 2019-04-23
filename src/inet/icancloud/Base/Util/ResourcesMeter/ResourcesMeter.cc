#include "ResourcesMeter.h"

namespace inet {

namespace icancloud {


Define_Module(ResourcesMeter);


void ResourcesMeter::initialize(){

	std::ostringstream osStream;

		// Module parameters
		timer = par ("timer");
		const char* fileName = par ("outputFile");

		// Latency message
		timerMsg = new cMessage ();

		// Creating output file
		if ((outputFile = fopen (fileName, "w+")) == nullptr){
			printf ("Error creating outputFile for <ResourceMeter>:%s\n", fileName);
			exit (0);
		}

		// Close file
		fclose (outputFile);

		// Command to calculate total memory!
		osStream << "free | grep Mem | awk '{print $2}' >> " << fileName;
		system (osStream.str().c_str());
		osStream.str("");

		// Command to get the global memory
		osStream << "ps aux | grep icancloud | grep -v grep | awk '{print $3 \"\\t\" $4}' >> " << fileName;
		consumptionCommand = osStream.str();

		// Set up the timer!
		scheduleAt (simTime(), timerMsg);
}



void ResourcesMeter::handleMessage(cMessage *msg){

	if (msg->isSelfMessage()){

		// Get memory consumption!
		system (consumptionCommand.c_str());

		// Set up timer again!
		scheduleAt (simTime()+timer, timerMsg);
	}
}



void ResourcesMeter::finish(){

}



} // namespace icancloud
} // namespace inet
