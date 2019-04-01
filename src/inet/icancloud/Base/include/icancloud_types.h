#ifndef _icancloud_types_H_
#define _icancloud_types_H_


#include "inet/icancloud/Base/include/icancloud_debug.h"
#include <omnetpp.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

/************************* Constants *************************/

// ---------- icancloud sizes and lengths ---------- //

/** Number of bytes per sector */
#define BYTES_PER_SECTOR 512

/** Kilo byte */
#define KB 1024u

/** Mega byte */
#define MB (KB*KB)

/** Giga byte */
#define GB (MB*KB)

/** Maximum name size. */
#define NAME_SIZE (1*KB)

/** Maximum line size. */
#define LINE_SIZE (2*KB)

/** Delay ratio for MPI_SEND operation */
#define DELAY_SEND_MB 200.0

/** Delay ratio for MPI_RECV operation */
#define DELAY_RECV_MB 600.0

/** Delay overhead for MPI_SEND operation */
#define DELAY_SEND_TA 0.075

/** Delay overhead for MPI_SEND operation */
#define DELAY_SEND_TB 0.005

/** Delay overhead for MPI_SEND operation */
#define DELAY_SEND_TC 0.00025

/** Minimum delay overhead for MPI_SEND operations */
#define DELAY_SEND_MINIMUM 0.000001


// ---------- icancloud return values and states ---------- //

/** All OK */
#define icancloud_OK 0

/** Error... */
#define icancloud_ERROR -1

/** File not found */
#define icancloud_FILE_NOT_FOUND 30001

/** There is not free space */
#define icancloud_DISK_FULL 30002

/** Requested data are out of bounds */
#define icancloud_DATA_OUT_OF_BOUNDS 30003

/** Empty. Not used */
#define EMPTY -2

/** Full. There is no free entry. */
#define NO_FREE_ENTRY -3

/** Something was not found. */
#define NOT_FOUND -4

/** No gate ID */
#define NO_GATE_ID -5

/** There is no free space... */
#define IS_FULL -6

/** Default server ID */
#define DEFAULT_SERVER_ID 0

/** Init Communication ID */
#define NULL_COMM_ID 99999999

/** nullptr Process ID */
#define NULL_PROCESS_ID 99999999

/** nullptr Sequence number */
#define NULL_SEQUENCE_NUMBER 999999999

/** MPI compute process */
#define MPI_COMPUTE_PROCESS "Compute"

/** MPI coordinator process */
#define MPI_COORDINATOR_PROCESS "Coordinator"

/** Default coordinator process rank */
#define DEFAULT_COORDINATOR_ID 0

/** Local file system identifier */
#define LOCAL_FS_TYPE "LOCAL"

/** Remote file system identifier */
#define REMOTE_FS_TYPE "REMOTE"

/** Delay message for MPI_SEND operations */
#define DELAY_SEND_MSG "delay-send-message"

/** Delay message for MPI_RECV operations */
#define DELAY_RECV_MSG "delay-recv-message"

/** Delay message for MPI_BARRIER operations */
#define DELAY_BARRIER_MSG "delay-barrier-message"

/** Delay message for MPI_BARRIER operations */
#define DELAY_BCAST_MSG "delay-bcast-message"


/************************* Typedefs *************************/

/** Offset in blockList */
typedef unsigned long long int off_blockList_t;

/** Branch size in blockList (Number of contiguous blocks) */
typedef unsigned long long int size_blockList_t;

/** Typedef to maintain compatibility with Mac OS */
typedef unsigned long long int off64_T;

/** Request number */
typedef unsigned long int reqNum_t;

/** Structs for parsing preload and ior */

		struct cfgPreload_t{
				string fileName;
				int fileSize;

		};
typedef struct cfgPreload_t preload_T;

		struct cfgFileSystem_t{
				string fsType;
				string fsRoute;
		};
typedef struct cfgFileSystem_t fsStructure_T;

/** Struct for defining the connection data */

		struct connection_t{
			// Parameters for identify the connection
				int uId;
				int pId;

			// Parameters that define the connection
				string ip;
				int port;
		};
typedef struct connection_t connection_T;

/** To log the states of the vms. Each state changed will generate a new vmStatesLog entry*/
    struct vmStatesLog{
        string vm_state;                          // The code of the state.
        double init_time_M;                        // when the new change state has begun (in Minutes)
    };
typedef vmStatesLog vmStatesLog_t;


/************************* Structs *************************/

/**
 * Struct that represent a file branch
 */
 struct FileBranch{
	off_blockList_t offset;			/**< Offset */
	size_blockList_t numBlocks;		/**< Branch size */
 };
 typedef struct FileBranch fileBranch;


/**
 * Structure that represents a process action.
 */
 struct MPI_ProcessAction{
 	unsigned int operation;				/** Operation type */
 	omnetpp::simtime_t cpuTime;					/** CPU time for processing */
 	string fileName;					/** File name */
 	unsigned int offset;				/** Offset */
 	unsigned int sizeKB;				/** Size (read/write) in KB */
	unsigned int firstProcessID;		/** First destination process ID */
	unsigned int lastProcessID;			/** Last destination process ID */
	unsigned int iteration;				/** Current iteration */
	unsigned int state;					/** Current state */
	int receivedBarrierAcks;			/** Number of received barrier acks */
	int receivedDataAcks;				/** Number of received data acks */
	int receivedResultAcks;				/** Number of received result acks */
	string error;						/** Error message */
 };
 typedef struct MPI_ProcessAction processAction;


/************************* Flags *************************/

/** To use large files */
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

namespace inet {

namespace icancloud {



/************************* Flags *************************/




} // namespace icancloud
} // namespace inet

#endif /*icancloudTYPES_H_*/
