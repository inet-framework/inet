#ifndef __APPLICATION_HPC_H_
#define __APPLICATION_HPC_H_

#include <omnetpp.h>
#include "inet/icancloud/Applications/Libraries_API/MPI_Base/MPI_Base.h"

namespace inet {

namespace icancloud {


/**
 * @class ApplicationHPC ApplicationHPC.h "ApplicationHPC.h"
 *
 * This Application models a generic behavior of a HPC application
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-13
 *
 * @author updated to iCanCloud by Gabriel González Castañé
 * @date 2012-05-17
 */

class ApplicationHPC : public MPI_Base{

	protected:
	
		unsigned int numIterations;
		unsigned int sliceToWorkers_KB;
		unsigned int sliceToMaster_KB;
		unsigned int sliceCPU;
		unsigned int currentIteration;
		unsigned int masterSet;
		
		bool createdResultsFile;
		int64_t totalResponses;
		unsigned int currentOffset;
		
		bool workersRead;
		bool workersWrite;
		
		char outFileName [NAME_SIZE];
		char inputFileName [NAME_SIZE];

		unsigned int offsetRead;
		unsigned int offsetWrite;

		unsigned int dataSet_total_KB;
		unsigned int dataResult_total_KB;
		unsigned int previousData_input_KB;
		unsigned int previousData_output_KB;

		unsigned int currentInputFile;
		unsigned int currentOutputFile;

		simtime_t totalIO;
		simtime_t ioStart;
		simtime_t ioEnd;

		simtime_t totalNET;
		simtime_t netStart;
		simtime_t netEnd;

		simtime_t totalCPU;
		simtime_t cpuStart;
		simtime_t cpuEnd;
		
		/** State */
		unsigned int currentState;

		
	   /**
		* Destructor
		*/
		~ApplicationHPC();

	   /**
 		*  Module initialization.
 		*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    

	   /**
 		* Module ending.
 		*/
	    virtual void finish() override;

	    /*
	     * Starts the app execution
	     */
	    virtual void startExecution() override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		void processSelfMessage (cMessage *msg) override;;
	   
	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;
		
	   /**
		* This function calculate next state of a given process.
		* 
		*/
		void calculateNextState ();	
		
	   /**
		* Process next iteration
		* 
		*/
		void nextIteration ();		
		
	   /**
		* This function will be invoked when current blocked process can continue its execution.		
		* 
		*/
		void continueExecution() override;


	private:	

	   void getRAM(bool isInit);
	
	   /**
		* Master reads the corresponding data from input dataset.
		*/
		void readInputData();	
		
		/**
		* Workers read the corresponding data from input dataset.
		*/
		void readInputDataWorkers();	
	
	   /**
		* Master process split data among worker processes.
		*/
		void deliverData();


		void allocating();
		
		/**
		* Master process waits for results sent from worker processes
		*/
		void receivingResults();		
	
	   /**
		* Worker processes receive data from master process
		*/
		void receiveInputData();		
	
	   /**
		* Worker processes processing CPU.
		*/
		void processingCPU();	
		
	   /**
		* Master process writes the data of current iteration
		*/
		void writtingData();	
		
	   /**
		* Workers processes write the data of current iteration
		*/
		void writtingDataWorkers();
		
	   /**
		* Worker processes send results to master process
		*/
		void sendResults();			
	
	   /**
		* Process a NET call response.
		* 
		* @param responseMsg Response message.
		*/
		void processNetCallResponse (Packet *);
		
		
	   /**
		* Process an IO call response.
		* 
		* @param responseMsg Response message.
		*/
		void processIOCallResponse (Packet *);
		
	   /**
		* Process a CPU call response.
		* 
		* @param responseMsg Response message.
		*/
		void processCPUCallResponse (Packet *);

       /**
        * Process a Memory call response.
        *
        * @param responseMsg Response message.
        */
		void processMEMCallResponse (Packet *);
	
		/**
		 * Parses a state to a string format.
		 * 
		 * @return State in string format.
		 */
		string stateToString(int state);
		
		/**
		 * Show results of the simulation
		 */
		void showResults ();
};

} // namespace icancloud
} // namespace inet

#endif
