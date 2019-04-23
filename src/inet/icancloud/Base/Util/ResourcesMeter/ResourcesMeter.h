#ifndef __RESOURCESMETER_H_
#define __RESOURCESMETER_H_

#include <omnetpp.h>
#include <string.h>

namespace inet {

namespace icancloud {

using std::string;

using namespace omnetpp;

/**
 * @class ResourcesMeter ResourcesMeter.h "ResourcesMeter.h"
 *
 * This module performs measures of resources like RAM, CPU, etc...
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class ResourcesMeter: public cSimpleModule{

	protected:

		/** File to save results */
		FILE* outputFile;

		/** Timer (in sec.) */
		unsigned int timer;

		/** Timer message (self message) */
		cMessage *timerMsg;

		/** Command to measure current memory and cpu consumption */
		string consumptionCommand;


	   /**
		*  Module initialization.
	 	*/
        virtual void initialize() override;
        

	   /**
		* This method classifies an incoming message and invokes the corresponding method
		* to process it.
		* @ param msg Incoming message.
		*/
		void handleMessage (cMessage *msg) override;

	   /**
	 	* Module ending.
	 	*/
	    void finish() override;
};

} // namespace icancloud
} // namespace inet

#endif
