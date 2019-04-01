#include "inet/icancloud/Base/Parser/cfgMPI.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;

CfgMPI::~CfgMPI(){
	if (processVector.size() > 0)
		processVector.clear();	
}


CfgMPI::CfgMPI (){
	processVector.clear();
}


void CfgMPI::parseFile(const char* fileName){

	FILE* machineFile_fd;
	char line [LINE_SIZE];
	int parsedParameters;
	unsigned int i;
	std::ostringstream info;
	cfgMPIProcess newProcess;

	unsigned int numProcess;
	char hostNameValue [LINE_SIZE];
	unsigned int portValue;
	unsigned int rankValue;
	

		// Init...
		memset (line, 0, LINE_SIZE);

		// Open the servers file
		machineFile_fd = fopen (fileName, "r");

		// There is no servers file
		if (machineFile_fd == nullptr){
			info << "Error opening MPI Trace Player config file:" << fileName;
			perror (info.str().c_str());
			throw cRuntimeError (info.str().c_str());
			throw cRuntimeError(info.str().c_str());
		}

		// Read the number of entries
		fgets (line, LINE_SIZE, machineFile_fd);
	    numProcess = atoi (line);

		// Read all lines!
		for (i=0; i<numProcess; i++){

			// Load the current path entry
			if (fgets (line, LINE_SIZE, machineFile_fd)==nullptr){
				info << "Error reading file " << fileName << ". Line: " << line;
				throw cRuntimeError(info.str().c_str());
			}

			// Parse the data
			else{

				// Clear buffers
				memset (hostNameValue, 0, LINE_SIZE);

				// Parses the line!
				parsedParameters = sscanf (line, "%[^':\n']:%u:%u", 
											hostNameValue,
											&portValue,
											&rankValue);

				// Well parsed!
				if (parsedParameters==3){
					newProcess.hostName = hostNameValue;
					newProcess.port = portValue;
					newProcess.rank = rankValue;
					processVector.push_back (newProcess);
				}

				// Error!
				else{
					info << "Error parsing file " << fileName << ". Line: " << line;
					throw cRuntimeError(info.str().c_str());
				}
	    	}
		}
}


void CfgMPI::insertMPI_node_config (string hostName, int port, int rank){
	cfgMPIProcess newProcess;

	newProcess.hostName = hostName;
	newProcess.port = port;
	newProcess.rank = rank;

	processVector.push_back (newProcess);
}


string CfgMPI::getHostName (int index){

    int size = processVector.size();

	if (index>=size)
		throw cRuntimeError("Index out of bounds in getId (getHostName)");
	else
		return processVector[index].hostName;
}


unsigned int CfgMPI::getPort (int index){

    int size = processVector.size();

	if (index>=size)
		throw cRuntimeError("Index out of bounds in getId (getPort)");
	else
		return processVector[index].port;
}


unsigned int CfgMPI::getRank (int index){

    int size = processVector.size();

	if (index>=size)
		throw cRuntimeError("Index out of bounds in getRank (getRank)");
	else
		return processVector[index].rank;
}


unsigned int CfgMPI::getNumProcesses (){
	
	return (processVector.size());	
}


string CfgMPI::toString(){
	
	int i;
	std::ostringstream info;

    int size = processVector.size();

		if (size == 0)
			info << "Machine file empty!" << endl;

		else{
			info << "Printing MPI environment (CfgMPI class)..." << endl;
			
			for (i=0; i<size; i++){

				info << "  Process [" << i << "]"  
					 << "  HostName:" << processVector[i].hostName 
					 << "  Port:" << processVector[i].port 
					 << "  ID:" << processVector[i].rank
					 << endl;
			}
		}
		
	return info.str();
	
}


} // namespace icancloud
} // namespace inet
