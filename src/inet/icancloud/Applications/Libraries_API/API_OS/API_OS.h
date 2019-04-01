#ifndef __APP_SYSTEM_REQUESTS_H_
#define __APP_SYSTEM_REQUESTS_H_

#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/Messages/icancloud_App_IO_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_CPU_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_NET_Message.h"
#include "inet/icancloud/Base/Messages/icancloud_App_MEM_Message.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

namespace icancloud {


/**
 * @class AppSystemRequests AppSystemRequests.h "AppSystemRequests.h"
 *
 * Base class used by applications. This class contains the Operating System calls interface.
 *
 * @updated to iCanCloud by Gabriel González Castañé
 * @date 2013-11-17
 *
 */
class API_OS: virtual public icancloud_Base{

	protected:

		//**  Timeout between each iteration of the aplication */
		simtime_t timeout;

		//**  holds pointer to the timeout self-message */
	    cMessage *timeoutEvent;

	   /**
		* Structure that represents a connection between two TCP applications.
		*/
		struct icancloud_App_Connector{			
			string localAddress;			/**< Local Address. */
			string destAddress;				/**< Destination Address. */
			int localPort;					/**< Local Port */
			int destPort;					/**< Destination Port */
			string connectionType;			/**< Connection type */
			int id;							/**< Local connection ID */
			int connectionId;				/**< Connection Id (for sockets) */
		};
		typedef struct icancloud_App_Connector connector;		

		/** Local IP */
		string appLocalIP;		
		
		/** Local port */
		int appLocalPort;
		
		/** Input gate from OS. */
		cGate* fromOSGate;

		/** Output gate to OS. */
		cGate* toOSGate;


	   /** connector vector that contains the corresponding data to establish connection with servers.
    	* Note: Must be initialized on derived classes.
    	*/
    	vector <connector> connections;


		// Flag to know if the application has to be finalized
		bool   flagFinalize;


	   /**
		* Destructor
		*/
	    ~API_OS();

	   /**
		* Module initialization.
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
        virtual void startExecution ();

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		virtual void processSelfMessage (cMessage *msg) override = 0;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		virtual void processRequestMessage (Packet *) override = 0;

	   /**
		* Process a response message from a module in the local node.
		* @param sm Request message.
		*/
		virtual void processResponseMessage (Packet *) override = 0;

       /**
        * Get the out Gate to the module that sent <b>msg</b>.
        * @param msg Arrived message.
        * @return Gate (out) to module that sent <b>msg</b> or nullptr if gate not found.
        */
        cGate* getOutGate (cMessage *msg) override;



		// ----------------- I/O ----------------- //
		
		/**
		 * Open a file.
		 * This function creates and sends the corresponding message to Operating System module.
		 * @param fileName Name of the corresponding file to open.
		 */
		void icancloud_request_open (const char* fileName);
		
		/**
		 * Close a file.
		 * This function creates and sends the corresponding message to Operating System module.
		 * @param fileName Name of the corresponding file to close.
		 */
		void icancloud_request_close (const char* fileName);
		
		/**
		 * Read a file.
		 * This function creates and sends the corresponding message to Operating System module.
		 * @param fileName Name of the corresponding file to be opened.
		 * @param offset Offset (in bytes)
		 * @param size Number of bytes to read.
		 */
		void icancloud_request_read (const char* fileName, unsigned int offset, unsigned int size);
		
		/**
		 * Close a file.
		 * This function creates and sends the corresponding message to Operating System module.
		 * @param fileName Name of the corresponding file to be opened.
		 * @param offset Offset (in bytes)
		 * @param size Number of bytes to read.
		 */
		void icancloud_request_write (const char* fileName, unsigned int offset, unsigned int size);
		
		/**
		 * Create a file.
		 * This function creates and sends the corresponding message to Operating System module.
		 * @param fileName Name of the corresponding file to create.
		 */
		void icancloud_request_create (const char* fileName);
		
		/**
		 * Delete a file.
		 * This function creates and sends the corresponding message to Operating System module.
		 * @param fileName Name of the corresponding file to delete.
		 */
		void icancloud_request_delete (const char* fileName);
		
		/*
		 * Change the state of the component
		 */
		void icancloud_request_changeState_IO (string newState, vector<int> devicesIndexToChange);
		


		// ----------------- CPU ----------------- //
		
		/**
		 * Request for CPU execution
		 * This function creates and sends the corresponding message to CPU module.
		 * @param MIs Million instructions to be executed.		 
		 */
		void icancloud_request_cpu (unsigned int MIs);
		
		
		/**
		 * Request for CPU execution
		 * This function creates and sends the corresponding message to CPU module.
		 * @param cpuTime Time to execute the current CPU request.		 
		 */
		void icancloud_request_cpuTime (simtime_t cpuTime);
		
		
		/*
		 * Change the state of the component
		 */
		void icancloud_request_changeState_cpu (string newState, vector<int> devicesIndexToChange);

		
		// ----------------- Memory ----------------- //
		
		/**
		 * Request for allocating memory.
		 * This function allocates a corresponding amount of memory.
		 * @param memorySize Amount of memory (in bytes) to be allocated.
		 * @param region Region of the memory.		 
		 */
		void icancloud_request_allocMemory (unsigned int memorySize, unsigned int region);
		
		
		/**
		 * Request for releasing memory.
		 * This function releases a corresponding amount of memory.
		 * @param memorySize Amount of memory (in bytes) to be released.
		 * @param region Region of the memory.		 
		 */
		void icancloud_request_freeMemory (unsigned int memorySize, unsigned int region);

		/*
		 * Change the state of the component
		 */
		void icancloud_request_changeState_memory (string newState);


	   // ----------------- Network ----------------- //

		/**
		 * Create a listen connection (for servers)
		 *
		 * @param localPort Port where communication will be listen
		 * @param type Communication type
		 */
		void icancloud_request_createListenConnection (int localPort);


		/**
		* Create a connection with the corresponding server with other users (for clients)
		*
		* This function create a connection with destAddress:destPort.
		* Parameter setConnectionId contains the serverId.
		* Parameter destination user name is only for connect with other destinations out of the user
		* Response message will contain the connectionDescriptor stored in parameter connectionId.
		*
		* @param destAddress Server address.
		* @param destPort Server port.
		* @param id Local connection ID.
		* @param destination user name.
		*/
		void icancloud_request_createConnection (string destAddress,
											   int destPort,
											   int id,
											   int destUserName);

		/**
		* Create a connection with the corresponding server (for clients)
		*
		* This function create a connection with destAddress:destPort.
		* Parameter setConnectionId contains the serverId.
		* Response message will contain the connectionDescriptor stored in parameter connectionId.
		*
		* @param destAddress Server address.
		* @param destPort Server port.
		* @param id Local connection ID.
		*/
		void icancloud_request_createConnection (string destAddress,
												  int destPort,
												  int id);
		
		/**
		 * Sends a packet through the network.
		 * @param sm Message to send
		 * @param id Local connection ID
		 */
		void icancloud_request_sendDataToNetwork (Packet *pkt, int id);
		
		/**
		 * Close a connection ..
		 *
		 * @param localPort Port where communication will be close
		 * @param id Local connection ID.
		 *
		 */
		void icancloud_request_closeConnection (int localPort, int id);
		
		/**
		 * This method set the established connection.
		 * 
		 * This method must be invoked when the response of a SM_CREATE_CONNECTION operation arrives.
		 * The response message will contain the corresponding connection descriptor stored in parameter communicationId.
		 * 
		 * @param sm Response message.
		 */
		void setEstablishedConnection (Packet *);
				
		/**
		 * Search a connection by ID
		 * @param id Connection ID
		 * @return Index of requested connection if id exists or NOT_FOUND in other case.
		 * 
		 */
		int searchConnectionById (int id);		
		
		/**
         * Search a connection by destination IP
         * @param id Connection by destination IP
         * @return Index of requested connection if id exists or NOT_FOUND in other case.
         *
         */
        int searchConnectionByIp (string ip);

	    /**
		 * Parses all connections info to string. 
		 * 
		 * @param printConnections If true, this functions parses communication to string, else, do nothing.		
		 * @return Connections info in string format.
		 */
		string connectionsToString (bool printConnections);		

		/*
		 * Change the state of the coponent
		 */
		void icancloud_request_changeState_network (string newState, vector<int> devicesIndexToChange);

		
};

} // namespace icancloud
} // namespace inet

#endif

