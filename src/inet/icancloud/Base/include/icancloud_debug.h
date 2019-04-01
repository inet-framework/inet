#ifndef _icancloud_DEBUG_H_
#define _icancloud_DEBUG_H_

#include <string>
using std::string;


/************************* GLOBAL FLAGS *************************/

/** Flag to show a complete message trace*/
#define PRINT_SM_TRACE 1


/** Flag to show messages in GUI */
#define SHOW_GUI_MESSAGES 0


/** Flag to show messages in console */
#define SHOW_CONSOLE_MESSAGES 1

/** Flag to write messages to file */
#define WRITE_MESSAGES_TO_FILE 0


/** Flag to enable the started modules messages */
#define SHOW_STARTED_MODULE_MESSAGES 0


/** Flag to enable the log messages */
#define SHOW_LOG_MESSAGES 0


/** Flag to enable the debug messages */
#define SHOW_DEBUG_MESSAGES 1


/** Flag to enable the debug messages */
#define SHOW_ERROR_MESSAGES 1


/************************* APPLICATIONS *************************/

//// TRACE_LOCAL_APPLICATION

	#define DEBUG_Application false			        // Shows basic debug messages
	#define DEBUG_MSG_BasicApplication false		// Shows all messages enterely (including all parameters)


//// TRACE_LOCAL_APPLICATION

	#define DEBUG_LocalApplication false			// Shows basic debug messages
	#define DEBUG_MSG_LocalApplication false		// Shows all messages enterely (including all parameters)


//// TRACE_APPLICATION_HPC

	#define DEBUG_ApplicationHPC false				// Shows basic debug messages
	#define DEBUG_MSG_ApplicationHPC false			// Shows all messages enterely (including all parameters)

//// TRACE_APPLICATION_HTC

    #define DEBUG_ApplicationHTC false              // Shows basic debug messages
    #define DEBUG_MSG_ApplicationHTC false          // Shows all messages enterely (including all parameters)


//// NFS_CLIENT

	#define DEBUG_NFS_Client false					// Shows basic debug messages
	#define DEBUG_MSG_NFS_Client false				// Shows all messages enterely (including all parameters)
	#define DEBUG_SMS_NFS_Client false				// Shows the contents of the SMS structure
	
//// PFS_CLIENT
	#define DEBUG_PFS_Client false					// Shows basic debug messages
	#define DEBUG_MSG_PFS_Client false				// Shows all messages enterely (including all parameters)
	#define DEBUG_SMS_PFS_Client false				// Shows the contents of the SMS structure
	

//// NFS_SERVER

	#define DEBUG_NFS_Server false					// Shows basic debug messages
	#define DEBUG_MSG_NFS_Server false				// Shows all messages enterely (including all parameters)

//// APP_SYSTEM_REQUESTS

	#define DEBUG_AppSystem false					// Shows basic debug messages
	#define DEBUG_MSG_AppSystem false				// Shows all messages enterely (including all parameters)

//// REMOTE STORAGE APP Debug
	#define DEBUG_RemoteStorage false					// Shows basic debug messages

//// MPI_APP_BASE

	#define DEBUG_MPIApp false						// Shows basic debug messages
	#define DEBUG_MSG_MPIApp false					// Shows all messages enterely (including all parameters)

//// Application Checkpoint

	#define DEBUG_ApplicationCheckpoint false		//Shows basic debug messages
	#define DEBUG_MSG_ApplicationCheckpoint false	// Shows all messages enterely (including all parameters)


/************************* FILE CONFIGURATION *************************/		
		
#define DEBUG_FILE_CONFIG_printServers  false
#define DEBUG_FILE_CONFIG_printPreload  false
#define DEBUG_FILE_CONFIG_printIORCfg  false
#define DEBUG_FILE_CONFIG_printMPIEnvCfg  false

namespace inet {

namespace icancloud {

		
		
/************************* DISKS *************************/

	#define DEBUG_Disk false							// Shows basic debug messages
	#define DEBUG_DETAILED_Disk false				// Shows detailed debug messages
	#define DEBUG_MSG_Disk false					// Shows all messages enterely (including all parameters)
	#define DEBUG_BRANCHES_Disk false				// Shows the block list of each request message


/************************* CPU SCHEDULERS *************************/

	#define DEBUG_CPU_Scheduler_FIFO false				// Shows basic debug messages
	#define DEBUG_MSG_CPU_Scheduler_FIFO false			// Shows all messages enterely (including all parameters)
	#define DEBUG_CPU_Scheduler_RR false				// Shows basic debug messages
	#define DEBUG_MSG_CPU_Scheduler_RR false			// Shows all messages enterely (including all parameters)

/************************* CPU Cores *************************/

	#define DEBUG_CPUcore false					// Shows basic debug messages
	#define DEBUG_MSG_CPUcore false				// Shows all messages enterely (including all parameters)


/************************* BASIC FILE SYSTEM *************************/

	#define DEBUG_Basic_FS false							// Shows basic debug messages
	#define DEBUG_DETAILED_Basic_FS false					// Shows detailed debug messages
	#define DEBUG_PARANOIC_Basic_FS false					// Shows the FS layout after each FS call
	#define DEBUG_MSG_Basic_FS false						// Shows all messages enterely (including all parameters)
	#define DEBUG_BRANCHES_Basic_FS false					// Shows the block list of each request message
	#define DEBUG_FS_Basic_Files false						// Shows the block list of each request message


/************************* PARALLEL FILE SYSTEM *************************/

	#define DEBUG_Parallel_FS false					// Shows basic debug messages
	#define DEBUG_DETAILED_Parallel_FS false			// Shows detailed debug messages
	#define DEBUG_MSG_Parallel_FS false				// Shows all messages enterely (including all parameters)
	#define DEBUG_SMS_Parallel_FS false				// Shows the contents of the current SMS request
	#define DEBUG_ALL_SMS_Parallel_FS false			// Shows the contents of all SMS
	

/************************* I/O REDIRECTOR *************************/

	#define DEBUG_IO_Rediretor false				// Shows basic debug messages
	#define DEBUG_MSG_IO_Rediretor false			// Shows all messages enterely (including all parameters)
	

/************************* MAIN MEMORY *************************/

	#define DEBUG_Main_Memory false					// Shows basic debug messages
	#define DEBUG_DETAILED_Main_Memory false		// Shows detailed debug messages
	#define DEBUG_MSG_Main_Memory false				// Shows all messages enterely (including all parameters)
	#define DEBUG_SMS_Main_Memory false				// Shows the contents of the current SMS request
	#define DEBUG_ALL_SMS_Main_Memory false			// Shows the contents of all SMS
	#define DEBUG_SHOW_CONTENTS_Main_Memory false	// Shows the contents of the complete memory
	
/************************* BASIC MAIN MEMORY *************************/

	#define DEBUG_Basic_Main_Memory false					// Shows basic debug messages
	#define DEBUG_DETAILED_Basic_Main_Memory false		// Shows detailed debug messages
	#define DEBUG_MSG_Basic_Main_Memory false				// Shows all messages enterely (including all parameters)

/************************* Cache Block Latencies MEMORY *************************/

	
	#define DEBUG_CacheBlock_Latencies_Memory false						// Shows basic debug messages
	#define DEBUG_DETAILED_CacheBlock_Latencies_Memory false			// Shows detailed debug messages
	#define DEBUG_MSG_CacheBlock_Latencies_Memory false			// Shows all messages enterely (including all parameters)
	#define DEBUG_SMS_CacheBlock_Latencies_Memory false			// Shows the contents of the current SMS request
	#define DEBUG_ALL_SMS_CacheBlock_Latencies_Memory false				// Shows the contents of all SMS
	#define DEBUG_SHOW_CONTENTS_CacheBlock_Latencies_Memory false		// Shows the contents of the complete memory



/************************* NETWORK SERVICES *************************/

	#define DEBUG_Network_Service false				// Shows basic debug messages
	#define DEBUG_MSG_Network_Service false			// Shows all messages enterely (including all parameters)
	#define DEBUG_TCP_Service_Client false			// Shows basic debug messages
	#define DEBUG_TCP_Service_MSG_Client false		// Shows all messages enterely (including all parameters)
	#define DEBUG_TCP_Service_Server false			// Shows basic debug messages
	#define DEBUG_TCP_Service_MSG_Server false		// Shows all messages enterely (including all parameters)


/************************* STORAGE MANAGERS *************************/

	#define DEBUG_Storage_Manager false				// Shows basic debug messages
	#define DEBUG_MSG_Storage_Manager false			// Shows all messages enterely (including all parameters)
	#define DEBUG_BRANCHES_Storage_Manager false	// Shows the block list of each request message
	#define DEBUG_SMS_Storage_Manager false			// Shows the contents of the current SMS request
	#define DEBUG_ALL_SMS_Storage_Manager false		// Shows the contents of all SMS



/************************* STORAGE SCHEDULER *************************/

	#define DEBUG_Storage_Scheduler false				// Shows basic debug messages
	#define DEBUG_MSG_Storage_Scheduler false			// Shows all messages enterely (including all parameters)
	#define DEBUG_BRANCHES_Storage_Scheduler false	    // Shows the block list of each request message
	#define DEBUG_SMS_Storage_Scheduler false			// Shows the contents of the current SMS request
	
/************************* CLOUD SCHEDULER *************************/

	#define DEBUG_CLOUD_SCHED false

/************************* HYPERVISOR SCHEDULER *************************/

	#define DEBUG_HYPERVISOR_BASE false
	#define DEBUG_BASIC_HYPERVISOR false
	#define DEBUG_WAITING_QUEUE false
	#define DEBUG_H_NETMANAGER_BASE false
	#define DEBUG_MSG_H_CPUMANAGER_Scheduler_FIFO false



} // namespace icancloud
} // namespace inet

#endif /*icancloudTYPES_H_*/
