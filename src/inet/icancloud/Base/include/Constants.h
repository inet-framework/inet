

// ------------------ Performance states CPU --------
#define INCREMENT_SPEED		   std::string("increment_speed")
#define DECREMENT_SPEED		   std::string("decrement_speed")

// ---------------MACHINE STATES --------------------

#define MACHINE_STATE_IDLE std::string("idle")
#define MACHINE_STATE_RUNNING std::string("running")
#define MACHINE_STATE_OFF std::string("off_state")


 /**  ---------------- Requests States -------------------*/
#define REQUEST_ERROR          -1
#define REQUEST_NOP             0
#define REQUEST_PENDING         1
#define REQUEST_SUCCESS         2
#define REQUEST_UNSUCCESSFUL    3

 /**  ---------------- Requests operation -------------------*/
#define NOT_REQUEST            -1           // This is the initial value of the request
#define REQUEST_RESOURCES       0           // This is a request for resources reservation
#define REQUEST_START_JOB       1           // This is a request for start a job
#define REQUEST_STORAGE         2           // this is the general request for storage
#define REQUEST_LOCAL_STORAGE   3           // This is the concrete request for local storage
#define REQUEST_REMOTE_STORAGE  4           // This is the concrete request for remote storage
#define REQUEST_FREE_RESOURCES  5           // This is a request for free resources
#define REQUEST_ABANDON_SYSTEM  6           // This request is invoked when a user left the system

// ---------------- Storage Operations -------------------
#define FS_NFS                                   std::string("NFS")
#define FS_PFS                                   std::string("PFS")
#define FS_LOCAL                                 std::string("LOCAL")

/** ---------------- Requests VM operation -------------------*/
#define REQUEST_START_VM        7           // This is a request for get virtual resources

// ---------------- VM operations -------------------
#define NOT_PENDING_OPS                          0
#define PENDING_STORAGE                          1
#define PENDING_SHUTDOWN                         2
#define PENDING_STARTUP                          3


//--------------------------DEBUG STATES --------------
//
//#define DEBUG_BASE_NODE		    	false
//#define DEBUG_CLOUDMANAGER_BASE		false
//#define DEBUG_BASIC_CLOUDMANAGER	false
//#define DEBUG_VMMAP					false
//#define DEBUG_HETEROGENEOUS_VMSET	false
//#define DEBUG_HYPERVISOR_BASE		false
//#define DEBUG_PARSER				false
//#define DEBUG_E_CPU_CORE			false
//#define DEBUG_E_MEMORY				false
//#define DEBUG_E_DISK				false
//#define DEBUG_MIGRATION_CONTROLLER  false

//SCHEDULING DECISIONS.-
// -------- H_MEMORYMANAGER ------------

#define EXECUTE_MEM_MSG				 0
#define NOT_ENOUGH_MEM				 1
#define RETURN_MESSAGE				 2
#define NOT_EXECUTE_MEM_MSG			 3
#define MIGRATION_RETURN			 4
#define TO_STORAGE_APP				 5


// -------- H_STORAGE_MANAGER ------------

#define EXECUTE_STORAGE_MSG				 0
#define NOT_EXECUTE_STORAGE_MSG		 1
#define SET_STORAGE_CONTENTS			 	 2

namespace inet {

namespace icancloud {

} // namespace icancloud
} // namespace inet

#define SET_STORAGE_MIGRATION_CONTENTS	 3


