#ifndef __CFG_MPI_
#define __CFG_MPI_

#include <omnetpp.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "inet/icancloud/Base/include/icancloud_types.h"

namespace inet {

namespace icancloud {




/**
 * @class CfgMPI cfgMPI.h "Base/Parser/cfgMPI.h"
 *
 * Class that contains all parameters for MPI_TracePlayer Apps
 *
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 02-10-2007
 */
class CfgMPI{
	

	struct cfgMPI_t{
		string hostName;
		unsigned int port;
		unsigned int rank;
	};
	typedef struct cfgMPI_t cfgMPIProcess;

	public:

		std::vector <cfgMPIProcess> processVector;


		CfgMPI ();
		~CfgMPI();
		void parseFile (const char* fileName);		
		void insertMPI_node_config (string hostName, int port, int rank);
		string getHostName (int index);
		unsigned int getPort (int index);
		unsigned int getRank (int index);	
		unsigned int getNumProcesses ();
		string toString ();
};

} // namespace icancloud
} // namespace inet

#endif
