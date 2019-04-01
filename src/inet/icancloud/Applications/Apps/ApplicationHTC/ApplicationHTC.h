#ifndef __APPLICATION_HTC_H_
#define __APPLICATION_HTC_H_

#include <omnetpp.h>
#include "inet/icancloud/Applications/Libraries_API/MPI_Base/MPI_Base.h"

namespace inet {

namespace icancloud {


/**
 * @class ApplicationHTC ApplicationHTC.h "ApplicationHTC.h"
 *
 * This Application models a generic behavior of a HTC application
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-13
 *
 * @updated to iCanCloud by Gabriel González Castañé
 * @date 2012-05-17
 *
 */
class ApplicationHTC : public MPI_Base{

	protected:		
	
		/** Start delay time */
	  	double startDelay_s;
		
		/** Number of files to be processed */
		unsigned int numProcessedFiles;
		
		/** Suffix of the file name */
		string fileNameSuffix;
		
		/** Size of each file */
		unsigned int fileSize_MB;
		
		/** Size of results */
		unsigned int resultsSize_MB;
		
		/** Size of each file */
		unsigned int cpuMIs;	
		
		/** State */
		unsigned int currentState;		
	
		/** Number of remaining files */
		unsigned int currentFile;	
	
		/** Simulation Starting timestamp */
		simtime_t simStartTime;

		/** Simulation Ending timestamp */
		simtime_t simEndTime;
		
		/** Running starting timestamp */
		time_t runStartTime;

		/** Running ending timestamp */
		time_t runEndTime;
		
	
		
	   /**
		* Destructor
		*/
		~ApplicationHTC();

	   /**
 		*  Module initialization.
 		*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    

        /*
         * Starts the app execution
         */
        virtual void startExecution() override;

	   /**
 		* Module ending.
 		*/
	    virtual void finish() override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		void processSelfMessage (cMessage *msg) override;;
		
		/**
		* Process a request message.
		* @param msg request message.
		*/
		void processRequestMessage (Packet *) override;
	   
	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;

       /**
        * This function will be invoked when current blocked process can continue its execution.
        *
        */
        void continueExecution () override;

	private:
	
	   /**
		* Execute next state. 
		*/
		void executeNextState();
		
	   /**
	    * Process next file
		*/
		void processNextFile();
		
	   /**
		* Open file
		*/
		void openFile();
		
	   /**
		* Master reads the corresponding data from input dataset.
		*/
		void readInputData();		
	
	   /**
		* Worker processes processing CPU.
		*/
		void processingCPU();	
		
	   /**
		* Master process writes the data of current iteration
		*/
		void writtingData();		
		
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
		* Get the current input file name
		* @param currentFileNumber Current file
		* @return Current file name
		*/
		string getCurrentFileName (unsigned int currentFileNumber);

	   /**
		* Get the results file name
		* @return Results file name
		*/
		string getResultsFileName();
			
		/**
		 * Show results of the simulation
		 */
		void showResults ();
};

} // namespace icancloud
} // namespace inet

#endif
