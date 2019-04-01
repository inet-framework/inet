#ifndef __CPU_INTENSIVE_H_
#define __CPU_INTENSIVE_H_

#include <omnetpp.h>
#include "inet/icancloud/Applications/Base/UserJob.h"

namespace inet {

namespace icancloud {


/**
 * @class CPU_Intensive CPU_Intensive.h "CPU_Intensive.h"
 *
 * Example of a sequential application without traces.
 * This application alternates I/O operations with CPU.
 *
 * This module needs the following NED parameters:
 *
 * - startDelay (Starting delay time)
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-13
 *
 * @author updated to iCanCloud by Gabriel González Castañé
 * @date 2013-12-17
 *
 */
class CPU_Intensive : public UserJob{

	
	protected:

		/** Size of data chunk to read in each iteration */
		int inputSizeMB;

		/** Size of data chunk to write in each iteration */
		int outputSizeMB;

		/** Number of Instructions to execute */
		int MIs;

        /** Number of iterations */
        unsigned int currentIteration;

        /** Total iterations */
        unsigned int iterations;

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
				
		/** Execute write operation */
		bool executeWrite;
		
		/** Read Offset */
		int readOffset;

		/** Write Offset */
		int writeOffset;
				
								
		
	   /**
		* Destructor
		*/
		~CPU_Intensive();

	   /**
 		*  Module initialization.
 		*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    

	   /**
 		* Module ending.
 		*/
	    virtual void finish() override;

        /**
         * Start the app execution.
         */
        virtual void startExecution () override;

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

	   /**
		* Method that creates and sends a new I/O request.
		* @param executeRead Executes a read operation
		* @param executeWrite Executes a write operation
		*/
		void executeIOrequest(bool executeRead, bool executeWrite);		

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
