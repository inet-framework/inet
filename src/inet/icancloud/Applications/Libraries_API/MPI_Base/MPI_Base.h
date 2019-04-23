#ifndef __MPI_BASE_H_
#define __MPI_BASE_H_


#include <omnetpp.h>
#include "inet/icancloud/Applications/Base/UserJob.h"

#include "inet/icancloud/Applications/Libraries_API/MPI_Base/MPI_Call.h"

namespace inet {

namespace icancloud {

using namespace std;


/**
 * @class AppMPI_Base AppMPI_Base.h "AppMPI_Base.h"
 *
 * Base class used by MPI applications.
 * 
 * This Base class manages the following parameters in the .ned file
 * 
 *  - allToAllConnections
 *  - myRank
 *  - connectionDelay_s
 *  - startDelay_s    
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 04-02-2010
 */
class MPI_Base: public UserJob{


	protected:
	
	   /**
		* Structure that represents a connection between two TCP applications.
		*/
		struct MPIApp_Connector{			
			string destAddress;				/**< Destination Address. */			
			int destPort;					/**< Destination Port */			
			int rank;						/**< Local connection ID */			
		};

		
		typedef struct MPIApp_Connector mpiConnector;
		
				
	   /** connector vector that contains the corresponding data to establish connection with another processes.    	
    	*/
    	vector <mpiConnector> mpiConnections;
		
		/** Connection between all processes? */
		bool allToAllConnections;
		
		/** Process Rank */
		unsigned int myRank;
		
		/** Total number of MPI processes */
	  	unsigned int numProcesses;

	  	/** Number of worker processes per set */
	  	int64_t workersSet;
		
		/** Computing delay message */
	  	cMessage *computingDelayMessage;		
	  	
	  	/** Connection delay time */
	  	simtime_t connectionDelay_s;
	  	
	  	/** Start delay time */
	  	simtime_t startDelay_s;
		
		/** Simulation Starting timestamp */
	  	simtime_t simStartTime;

		/** Simulation Ending timestamp */
	  	simtime_t simEndTime;

		/** Running starting timestamp */
		time_t runStartTime;

		/** Running ending timestamp */
		time_t runEndTime;  
		
		/** Current MPI call */
		MPI_Call *currentMPICall;
		
		/** Current MPI call */
		MPI_Call *lastSynchroMPICall;

	  	/** Sync barrier to all processes */
		int *barriers;
		
		/** Cache list */
	    list <MPI_Call*> mpiCallList;
	
	    /** mpi config*/
	    CfgMPI *mpiCfg;

	   /**
		* Destructor
		*/
		~MPI_Base();

	   /**
 	 	*  Module initialization.
  		*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    

	   /**
		* Module ending.
		*/
		void finish() override;

        /**
         * Start the app execution.
         */
        virtual void startExecution () override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		virtual void processSelfMessage (cMessage *msg) override;;
		
	   /**
		* Process a response message from a module in the local node. Empty method.
		* All response messages are computed by TcpSocket::CallbackInterface
		* @param sm Request message.
		*/
		virtual void processResponseMessage (Packet *) override = 0;
		

	   /**
		* This function will be invoked when current blocked process can continue its execution.
		*
		*/
		virtual void continueExecution () = 0;


	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		virtual void processRequestMessage (Packet *) override;
		
		
	   /**
		* Process a MPI call request.
		*/
		void processMPICallRequest (Packet *);
		
		
	   /**
		* Parse the machines File.
		* This method set the attributes "myRank", "numProcess" and "numSockets".
		*
		*/
		void parseMachinesFile ();


		/**
		 *
		 * Checks whether a process is a master process
		 *
		 * @param processRank Rank of the process to be checked
		 * @return True if current process is a master process, or false in another case.
		 *
		 */
		bool isMaster (int processRank);
		
		
		int getMyMaster (int processRank);


		bool isThisWorkerInMyRange (int master, int processRank);


 	   /**
 		* Executes a MPI_SEND call.
 		* @param rankReceiver Rank of the process that receive data.
 		* @param bufferSize Number of sent bytes.
 		*/
 		virtual void mpi_send (unsigned int rankReceiver, int bufferSize);
 		
 		
 		/**
 		 *
 		 *
 		 */
 		bool isProcessInThisNode (unsigned int processID);

 	   /**
 		* Executes a MPI_RECV call.
 		* @param rankSender Rank of the process that send data.
 		* @param bufferSize Number of sent bytes.
 		*/
 		virtual void mpi_recv (unsigned int rankSender, int bufferSize);

        /**
         * Executes a MPI_IRECV call (asyncronous).
         * @param rankSender Rank of the process that send data.
         * @param bufferSize Number of sent bytes.
         */
         virtual void mpi_irecv (unsigned int rankSender, int bufferSize);


 	   /**
 		* Executes a MPI_BARRIER call.
 		*/
 		virtual void mpi_barrier ();

 	   /**
 		* Process a MPI_BARRIER_ACK Message.
 		* This function enables to process that performed MPI_BARRIER to continue its execution.
 		*/
 		virtual void mpi_barrier_up ();

 	   /**
 		* Process a MPI_BARRIER_DOWN Message.
 		* This function is executed only by MPI_MASTER_RANK. Its purpose is to syncronize all process.
 		* @param sm_mpi BARRIER_DOWN message.
 		*/
 		virtual void mpi_barrier_down (Packet *);
 		
 	   /**
 		* Init barriers.
 		*/
 		virtual void initBarriers (); 		
 		
	   /**
		* Check barriers for all processes
		*/
		virtual void checkBarriers ();	
 		
 	   /**
 		* Executes a MPI_BCAST call.
 		* @param root Rank of the process that splits and sends data.
 		* @param bufferSize Number of sent bytes to each process.
 		*/
 		virtual void mpi_bcast (unsigned int root, int bufferSize);		

 	   /**
 		* Executes a MPI_SCATTER call.
 		* @param root Rank of the process that splits and sends data.
 		* @param bufferSize Number of sent bytes to each process.
 		*/
 		virtual void mpi_scatter (unsigned int root, int bufferSize);
 		
 	   /**
 		* Executes a MPI_GATHER call.
 		* @param root Rank of the process that splits and sends data.
 		* @param bufferSize Number of sent bytes to each process.
 		*/
 		virtual void mpi_gather (unsigned int root, int bufferSize);
 		
 	   /**
 		* Process an incoming MPI_GATHER message.
 		* @param sm Message that contains a MPI_GATHER message.
 		*/
 		virtual void mpi_gather_arrives (Packet *);

	   /**
		* Synchronize current MPI call with incoming MPI Call
		*/
		virtual void synchronize (bool async);

	   /**
	    * Inserts a MPI call on list and sends a corresponding ACK message to sender process.
		* @param sm_mpi Message that contains the MPI call
		*/
		virtual void insertMPICall (Packet *);
		
	   /**
		* Search a concrete MPI incoming call on list.
		* @param call MPI call type
		* @param sender Corresponding sender process
		* @param size Number of bytes sent
		* @return Incoming call to search. If not found, nullptr value will be returned.
		*/
		virtual MPI_Call* searchMPICall(unsigned int call, unsigned int sender, int size);
		
	   /**
		* Deletes a MPI Call
		* @param removedCall MPI Call to remove
		*/
		virtual void deleteMPICall(MPI_Call* removedCall);		

	   /**
		* Parses all received MPI Calls to string.
		* This functions has debug purposes only.
		* @return MPI received calls in string format.
		*/
		string incomingMPICallsToString ();
 		
	   /**
		* Parses a MPI call to string format.
		* @param call MPI call.
		* @return MPI call in string format.
		*/
 		string callToString (int call);


	   /**
		* Add a new connection.
		* 
		* @param hostName Hostname where is executing the process to establish connection with.
		* @param port Destination port.
		* @param rank Rank of the process to establish connection with.
		*		
		*/	
		void addNewConnection (string hostName, unsigned int port, unsigned int rank);
		
		
	   /**
		* Establish all pre-configured connections with corresponding processes
		*/
		void establishAllConnections ();
		

	   /**
		* Parses all MPI commincations to a string.
		* 
		* @return String that contains all MPI communications.
		*/
		string mpiCommunicationsToString ();
		
		
		/**
		 * Calculate the number of processes to send a BCast message
		 */
		unsigned int calculateBcastMax ();


		/**
		 * Returns the last synchronized call
		 *
		 * @return Last synchronized call
		 */
		MPI_Call* getLastSynchronizedCall ();


};

} // namespace icancloud
} // namespace inet

#endif
