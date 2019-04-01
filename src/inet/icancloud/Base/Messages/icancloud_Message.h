#ifndef _icancloud_MESSAGE_H_
#define _icancloud_MESSAGE_H_

#include "inet/common/packet/Packet.h"
#include "icancloud_Message_m.h"
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
using std::string;
using std::pair;
using std::vector;

static const bool DEBUG_PACKING = false;


/** No value operation */
static const int SM_NO_VALUE_OPERATION = 0;

/** nullptr operation (init) */
static const int NULL_OPERATION = 0;

/** Infinite quantum */
static const int INFINITE_QUANTUM = -1;

// ---------- IO Operations ---------- //

/** icancloud Message Delete all Files from user operation */
static const int SM_DELETE_USER_FS = 9;

/** icancloud Message Open File operation */
static const int SM_OPEN_FILE = 10;

/** icancloud Message Close File operation */
static const int SM_CLOSE_FILE = 11;

/** icancloud Message Read File operation */
static const int SM_READ_FILE  = 12;

/** icancloud Message Write File operation */
static const int SM_WRITE_FILE = 13;

/** icancloud Message Create File operation */
static const int SM_CREATE_FILE = 14;

/** icancloud Message Delete File operation */
static const int SM_DELETE_FILE = 15;

/** icancloud Message Change State */
static const int SM_CHANGE_DISK_STATE = 16;

/** icancloud Message Configure HBS_MANAGER to remote storage */
static const int SM_SET_HBS_TO_REMOTE = 17;

/** icancloud Message Configure HBS_MANAGER to remote storage */
static const int SM_ALLOCATE_MIGRATION_CONTENTS = 18;

/** icancloud Message Configure HBS_MANAGER to remote storage */
static const int SM_REMOVE_MIGRATION_CONTENTS = 19;

/** icancloud Message Notify preload finalization to cloud manager */
static const int SM_NOTIFY_PRELOAD_FINALIZATION = 20;

/** icancloud Message Notify preload finalization to cloud manager */
static const int SM_NOTIFY_USER_FS_DELETED = 21;

/** icancloud Message Notify the finalization of disconnecting network connections of VM */
static const int SM_NOTIFY_USER_CONNECTIONS_CLOSED = 22;

// ---------- MEM Operations ---------- //

/** icancloud Message Allocate memory */
static const int SM_MEM_ALLOCATE = 50;

/** icancloud Message Release memory */
static const int SM_MEM_RELEASE = 51;

/** icancloud Message Release memory */
static const int SM_MEM_SEARCH = 52;

/** icancloud Message Change State */
static const int SM_CHANGE_MEMORY_STATE = 53;

// ---------- CPU Operations ---------- //

/** icancloud Message Execution (CPU) */
static const int SM_CPU_EXEC = 70;

/** icancloud Message Change State */
static const int SM_CHANGE_CPU_STATE = 71;

// ---------- Net Operations ---------- //

/** icancloud Message Create connection */
static const int SM_CREATE_CONNECTION = 200;

/** icancloud Message Listen for connection */
static const int SM_LISTEN_CONNECTION = 201;

/** icancloud Message Send data trough network */
static const int SM_SEND_DATA_NET = 202;

/** icancloud Message Change State */
static const int SM_CHANGE_NET_STATE = 203;

/** icancloud Message Error, port not open ..*/
static const int SM_ERROR_PORT_NOT_OPEN = 204;

/** icancloud Message to close a single connection*/
static const int SM_CLOSE_CONNECTION = 205;

/** icancloud Message Error, to close all the vm connections ..*/
static const int SM_CLOSE_VM_CONNECTIONS = 206;


// --------------------- Remote storage App -------------- //

/** icancloud Message Change State */
static const int SM_VM_REQUEST_CONNECTION_TO_STORAGE = 410;

/** icancloud Message to create the file system data */
static const int SM_SET_IOR = 411;

/** icancloud Message to create a listen connection */
static const int SM_MIGRATION_REQUEST_LISTEN = 412;

/** icancloud Message to activate the remote storage for a vm */
static const int SM_UNBLOCK_HBS_TO_REMOTE = 413;

/** icancloud Message close a connection */
//static const int SM_CLOSE_CONNECTION = 414;


// Migration operations

/** icancloud Message Migrate a VM */
static const int SM_NODE_REQUEST_CONNECTION_TO_MIGRATE = 415;

/** icancloud Message close a connection */
static const int SM_VM_ACTIVATION = 416;

/** icancloud Message close a connection */
static const int SM_CONNECTION_CONTENTS = 427;

/** icancloud Message iterative pre-copy */
static const int SM_ITERATIVE_PRECOPY = 417;

/** icancloud Message to stop a virtual machine */
static const int SM_STOP_AND_DOWN_VM = 418;

/** icancloud Message to set the migration connections */
static const int SET_MIGRATION_CONNECTIONS = 419;

/** icancloud Message to set disk and memory contents */
static const int ALLOCATE_MIGRATION_DATA = 420;

/** icancloud Message to set disk and memory contents */
static const int REACTIVATE_REMOTE_CONNECTIONS = 421;

/** icancloud Message to set the migration connections */
static const int GET_MIGRATION_DATA = 422;

/** icancloud Message to get the migration connections */
static const int GET_MIGRATION_CONNECTIONS = 423;

// Identifiers to remote storage app from migration contents

/** icancloud Message to get the migration contents */
static const int STORAGE_DATA = 424;

/** icancloud Message to get the migration connections */
static const int STORAGE_CONNECTIONS = 425;

/** icancloud Message to get the migration connections */
static const int MEMORY_DATA = 426;

/** operation overhead*/
static const int VM_OVERHEAD = 427;

// ---------- MPI Operations (Trace Method) ---------- //

/** MPY any sender process */
#define MPI_ANY_SENDER 4294967294U

/** MPI No value */
#define MPI_NO_VALUE 999999999

/** MPI Master process Rank */
#define MPI_MASTER_RANK 0

/** MPI Send */
#define MPI_SEND 100

/** MPI Receive */
#define MPI_RECV 200

/** MPI Barrier */
#define MPI_BARRIER 300

/** Barrier Up */
#define MPI_BARRIER_UP 301

/** Barrier Down */
#define MPI_BARRIER_DOWN 302

/** MPI Broadcast */
#define MPI_BCAST 400

/** MPI Scatter */
#define MPI_SCATTER 500

/** MPI Gather */
#define MPI_GATHER 600

/** MPI File Open */
#define MPI_FILE_OPEN 700

/** MPI File Close */
#define MPI_FILE_CLOSE 800

/** MPI File Create */
#define MPI_FILE_CREATE 900

/** MPI File Delete */
#define MPI_FILE_DELETE 1000

/** MPI File Read */
#define MPI_FILE_READ 1100

/** MPI File Write */
#define MPI_FILE_WRITE 1200

namespace inet {

namespace icancloud {




// ---------- MPI Operations (Graph Method) ---------- //

/** MPI nullptr operation */
static const unsigned int MPI_nullptr = 100;

/** MPI error */
static const unsigned int MPI_ERROR = 110;

/** MPI Barrier */
static const unsigned int MPI_BARRIER_GO = 120;

/** MPI Barrier Ack */
static const unsigned int MPI_BARRIER_ACK = 130;

/** MPI Processing (cpu) */
static const unsigned int MPI_PROCESS = 140;

/** MPI Sending Metadata */
static const unsigned int MPI_METADATA = 150;

/** MPI Metadata Ack */
static const unsigned int MPI_METADATA_ACK = 160;

/** MPI Sinding data */
static const unsigned int MPI_DATA = 170;

/** MPI Data Ack */
static const unsigned int MPI_DATA_ACK = 180;

/** MPI Sending results */
static const unsigned int MPI_RESULT = 190;

/** MPI Results Ack */
static const unsigned int MPI_RESULT_ACK = 200;

/** MPI Send Barrier Ack and Read data */
static const unsigned int MPI_BARRIER_ACK_AND_READ_DATA = 210;

/** Process data and send data ack */
static const unsigned int MPI_PROCESS_AND_SEND_DATA_ACK = 220;

/** Send results and read data */
static const unsigned int MPI_SEND_RESULT_AND_READ_DATA = 230;

/** Send results ack and read data */
static const unsigned int MPI_SEND_RESULT_ACK_AND_READ_DATA = 240;




// ---------- NFS2 Message sizes---------- //

/** NFS message (open Request operation) length */
static const int64_t SM_NFS2_OPEN_REQUEST = 36;

/** NFS message (open Response operation) length */
static const int64_t SM_NFS2_OPEN_RESPONSE = 104;

/** NFS message (close Request operation) length */
static const int64_t SM_NFS2_CLOSE_REQUEST = 36;

/** NFS message (close Response operation) length */
static const int64_t SM_NFS2_CLOSE_RESPONSE = 4;

/** NFS message (read Request operation) length */
static const int64_t SM_NFS2_READ_REQUEST = 44;

/** NFS message (read Response operation) length */
static const int64_t SM_NFS2_READ_RESPONSE = 80;

/** NFS message (write Request operation) length */
static const int64_t SM_NFS2_WRITE_REQUEST = 52;

/** NFS message (write Response operation) length */
static const int64_t SM_NFS2_WRITE_RESPONSE = 72;

/** NFS message (create Request operation) length */
static const int64_t SM_NFS2_CREATE_REQUEST = 68;

/** NFS message (create Response operation) length */
static const int64_t SM_NFS2_CREATE_RESPONSE = 104;

/** NFS message (delete Request operation) length */
static const int64_t SM_NFS2_DELETE_REQUEST = 36;

/** NFS message (delete Response operation) length */
static const int64_t SM_NFS2_DELETE_RESPONSE = 4;





// ---------- icancloud Message Memory regions ---------- //

/** Memory region for code */
static const int SM_MEMORY_REGION_CODE = 61;

/** Memory region for local variables */
static const int SM_MEMORY_REGION_LOCAL_VAR = 62;

/** Memory region for global variables */
static const int SM_MEMORY_REGION_GLOBAL_VAR = 63;

/** Memory region for dynamic variables */
static const int SM_MEMORY_REGION_DYNAMIC_VAR = 64;

/** Memory error! Not enough memory! */
static const int SM_NOT_ENOUGH_MEMORY = 65;




// ---------- icancloud Message types ---------- //

/** Message to wait for the connection step*/
static const string SM_WAIT_TO_CONNECT = "Wait_To_Connect";

/** Message to wait for the execution step*/
static const string SM_WAIT_TO_EXECUTE = "Wait_To_Execute";

/** Message to wait for the execution step*/
static const string SM_LATENCY_MESSAGE = "latency-message";

/** Message to wait for the execution step*/
static const string SM_CHANGE_STATE_MESSAGE = "change_state_message";

/** Message to wait for the execution step*/
static const string SM_WAIT_TO_SCHEDULER = "Wait_for_Scheduler_message";

/** Message to eliminate the modules dynamically*/
static const string SM_DYNAMIC_MODULE_DELETION = "module_dynamic_deletion";

/** Message to collect all the energy values from the components*/
static const string FINALIZATION_SIMULATION = "finalization_simulation_message";

/** Message to activate a new vm migration in the migration app*/
static const string SM_NEW_MIGRATION = "migration_activation_message";

/** Message to listen for possible connection from other nodes*/
static const string SM_APP_ALARM = "app_alarm";

static const string SM_CALL_INITIALIZE = "call_initialize";

static const string SM_INIT_JOB = "init_job";

static const string SM_FINISH_JOB = "finish_job";

static const string SM_SUPER = "super_class";

static const string SM_NOTIFICATION = "notification";

// ----------- icancloud User Message Types ----------------//

static const string SM_NODE_CHANGE_STATE = "node_change_state";

static const string SM_INIT_HYPERVISOR = "node_hypervisor_initialization";

static const string SM_EXECUTE_APPS_STATE = "execute_applications_state";

// ---------- icancloud Message Initial Length ---------- //

/** Initial message length*/
static const int64_t MSG_INITIAL_LENGTH = 1;



/**
 * @class icancloud_Message icancloud_Message.h "Base/Messages/icancloud_Message.h"
 *
 * Class that represents a icancloud_Message.
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class icancloud_Message: public icancloud_Message_Base{

	protected:

		/** Pointer to parent request message or nullptr if has no parent */
		inet::Packet *parentRequest;

		/** Message trace */
		vector <pair <string, vector<TraceComponent> > > trace;

		string state;

		vector <int> change_states_index;

	public:


	   /**
		* Destructor.
		*/
		virtual ~icancloud_Message();


	   /**
		* Constructor of icancloud_Message
		* @param name Message name
		* @param kind Message kind
		*/
		icancloud_Message ();
		icancloud_Message (const char *) : icancloud_Message (){}

		virtual bool getIsResponse() const {return isResponse();}


	   /**
		* Constructor of icancloud_Message
		* @param other Message
		*/
		icancloud_Message(const icancloud_Message& other);


	   /**
		* = operator
		* @param other Message
		*/
		icancloud_Message& operator=(const icancloud_Message& other);


	   /**
		* Method that makes a copy of a icancloud_Message
		*/
		virtual icancloud_Message *dup() const override;

	   /**
		* Empty method
		*/
		virtual void setTraceArraySize(size_t size) override;


       /**
		* Empty method
		*/
    	virtual size_t getTraceArraySize() const override;

       /**
		* Empty method
		*/
    	virtual TraceComponent& getTrace(size_t k) const override;

        virtual void insertTrace(const TraceComponent& trace) override;
        virtual void insertTrace(size_t k, const TraceComponent& trace) override;
        virtual void eraseTrace(size_t k) override;

       /**
		* Empty method
		*/
    	virtual void setTrace(size_t k, const TraceComponent& trace_var) override;


    	void setChangingState (string newState);


    	string getChangingState () const;


    	void add_component_index_To_change_state (int componentIndex);


    	int get_component_to_change_size () const;


    	int get_component_to_change (int componentsPosition) const;

	   /**
		* Add a module to message trace
		* @param module Added mdule.
		* @param gate Gate ID to <b>module</b>.
		* @param currentRequest Current request number.
		*/
		void addModuleToTrace (int module, int gate, reqNum_t currentRequest);


	   /**
		* Removes the last module from the message trace, including all its request numbers.
		* If last trace component is not a Module, then return an empty string.
		*/
		void removeLastModuleFromTrace ();


	   /**
		* Add a node to message trace.
		* @param node Added node.
		*/
		void addNodeToTrace (string node);


	   /**
		* Removes the last node from message trace, including all its modules and request numbers.
		*/
		void removeLastNodeFromTrace ();


	   /**
		* Add a request to message trace.
		* @param request Added request.
		*/
		void addRequestToTrace (int request);


	   /**
		* Removes last request from message trace.
		*/
		void removeLastRequestFromTrace ();


	   /**
		* Get the las Gate ID from message trace.
		* @return If all OK, the return last gate ID, else return icancloud_ERROR
		*/
		int getLastGateId () const;


	   /**
		* Get the las module ID from message trace.
		* @return If all OK, the return last module ID, else return icancloud_ERROR
		*/
		int getLastModuleId () const;


	   /**
		* Get the current request.
		* @return Current request or icancloud_ERROR if an error occurs.
		*/
		int getCurrentRequest () const;


	   /**
		* Get the parent request message
		* @return Pointer to parent request message or nullptr if not exists a parent message.
		*/
		inet::Packet * getParentRequest () const;


	   /**
		* Set the parent request message
		* @param parent Pointer to parent request message.
		*/
		void setParentRequest (inet::Packet * parent);


	   /**
		* Parse current trace to string
		* @return String with the corresponding trace.
		*/
		string traceToString () const;


	   /**
		* Parse all parameters of current message to string.
		* @param printContents Print message contents.
		* @return String with the corresponding contents.
		*/
		virtual string contentsToString (bool printContents) const;

	   /**
		* Parse a icancloud_Message operation to string.
		* @param operation icancloud_Message operation
		* @return Operation icancloud_Message in string format.
		*/
		string operationToString (unsigned int operation) const;

	   /**
		* Parse a icancloud_Message operation to string.
		* @return Operation icancloud_Message in string format.
		*/
		string operationToString () const;
		
	   /**
		* Return if this is response message in string format.
		* @return "true" if this is a response message or "false" in other case.
		*/
		string getIsResponse_string () const;

	   /**
		* Return if this message contains a remote operation in string format.
		* @return "true" if this message contains a remote operation or "false" in other case.
		*/
		string getRemoteOperation_string () const;

	   /**
		* Calculates if message trace is empty.
		* @return True if message trace is empty or false in another case.
		*/
		bool isTraceEmpty () const;

	   /**
		* Method that calculates if current subRequest fits with its parent trace.
		*
		*/
	   	bool fitWithParent () const;

	   /**
	   	* Method that calculates if this message is son of <b>parentMsg</b>.
	   	* @return True if current message if son of parentMsg, or false in another case.
	   	*/
	   	bool fitWithParent (inet::Packet *parentMsg);

	   /**
	    * Add a node to trace
	    * @param host Hostname
	    * @param nodeTrace Node trace
	    */
		void addNodeTrace (string host, vector<TraceComponent> nodeTrace);

	   /**
		* Get the hostname of node k
		* @param k kth Node
		* @return hostname of kthe node
		*/
		string getHostName (int k) const;

	   /**
		* Get the kthe traceNode
		* @param k kth node
		* @return kth node trace
		*/
		vector<TraceComponent> getNodeTrace (int k) const;

	   /**
		* Serializes a icancloud_Message.
		* @param b Communication buffer
		*/
	    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;

	   /**
		* Deserializes a icancloud_Message.
		* @param b Communication buffer
		*/
	    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

} // namespace icancloud
} // namespace inet

#endif
