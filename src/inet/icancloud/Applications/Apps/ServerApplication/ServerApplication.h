#ifndef __SERVER_APPLICATION_H_
#define __SERVER_APPLICATION_H_

#include <omnetpp.h>
#include "inet/icancloud/Applications/Base/UserJob.h"


/**
 * @class ServerApplication ServerApplication.h "ServerApplication.h"
 *
 * Example of a sequential application without traces.
 * This application alternates I/O operations with CPU.
 *
 * This module needs the following NED parameters:
 *
 * - startDelay (Starting delay time)
 * 
 * @author Gabriel González Castañé
 * @date 2013-12-1
 *
 */

#define DEBUG true

namespace inet {

namespace icancloud {


class ServerApplication: public UserJob{
	
	protected:

		/** Size of data chunk to read in each iteration */
		int inputSize;

		/** Number of Instructions to execute */
		int MIs;

		/** Starting time delay */
		simtime_t startDelay;

		/** Simulation Starting timestamp */
		simtime_t simStartTime;

		/** Simulation Ending timestamp */
		simtime_t simEndTime;
		
		/** Running starting timestamp */
		time_t runStartTime;

		/** Running ending timestamp */
		time_t runEndTime;		
				
		/** Call Starting timestamp (IO) */
		simtime_t startServiceIO;
		
		/** Call Ending timestamp (IO) */
		simtime_t endServiceIO;
		
		/** Call Starting timestamp (CPU) */
		simtime_t startServiceCPU;
		
		/** Call Ending timestamp (CPU) */
		simtime_t endServiceCPU;
		
		/** Spent time in CPU system */
		simtime_t total_service_CPU;
		
		/** Spent time in IO system */
		simtime_t total_service_IO;
				
		/** Execute CPU */
		bool executeCPU;
		
		/** Execute read operation */
		bool executeRead;
		
		/** Read Offset */
		int readOffset;

        /** Uptime limit*/
        double uptimeLimit;
        int hitsPerHour;
        cMessage *newIntervalEvent;
        double intervalHit;
        int pendingHits;


	   /**
		* Destructor
		*/
		~ServerApplication();

	   /**
 		*  Module initialization.
 		*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    

        /**
         * Start the app execution.
         */
	    void startExecution() override;

	   /**
 		* Module ending.
 		*/
	    virtual void finish() override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		void processSelfMessage (cMessage *msg) override;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		void processRequestMessage (Packet *) override;

		/**
		* Process a message from the cloudManager or the scheduler...
		* @param sm message.
		*/
		void processSchedulingMessage (cMessage *msg);

	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;


		void changeState(string newState);

	private:			    

		void newHit();
	   /**
		* Method that creates and sends a new I/O request.
		* @param executeRead Executes a read operation
		* @param executeWrite Executes a write operation
		*/
		void serveWebCode();

	   /**
		* Method that creates and sends a CPU request.
		*/
		void executeCPUrequest();

	   /**
		* Print results.
		*/
		void printResults();

};

} // namespace icancloud
} // namespace inet

#endif
