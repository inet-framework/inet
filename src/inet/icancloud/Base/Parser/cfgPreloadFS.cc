#include "inet/icancloud/Base/Parser/cfgPreloadFS.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;

CfgPreloadFS::~CfgPreloadFS (){
	
	if (preloadVector.size() > 0)
		preloadVector.clear();
}


CfgPreloadFS::CfgPreloadFS (){

	preloadVector.clear();
}


void CfgPreloadFS::parseFile(const char* preLoad_data){

	char line [LINE_SIZE];
	int parsedParameters, i;
	
	unsigned int numPreloadFiles;
	unsigned int sizeValue;
	char fileNameValue [LINE_SIZE];
	
	preload_T preloadFile;
	std::ostringstream info;
	
	vector<string> config;
	
		// Parse the env
			config.clear();
			config = divide(preLoad_data);

		// There is no config file
		if (config.size() == 0){
			info << "Error opening preload defined in basic file system: " << preLoad_data;
			perror (info.str().c_str());
			throw cRuntimeError (info.str().c_str());
			throw cRuntimeError(info.str().c_str());
		}

		// start to load files...
		else{

			// Init...
			memset (line, 0,LINE_SIZE);

			// Read the number of files to load
			strcpy(line,(*config.begin()).c_str());
			config.erase (config.begin());
			numPreloadFiles = atoi (line);

			// Preload numPreloadFiles files!!!
			for (i=0; i<((int)numPreloadFiles); i++){
				
				// Load the current path entry

				strcpy(line,(*config.begin()).c_str());
				config.erase (config.begin());

				if (line == NULL){
					info << "Error parsing preload defined in basic file system: " << line;
					throw cRuntimeError(info.str().c_str());
				}

				else{
				
					// Parse current line
					memset (fileNameValue, 0, LINE_SIZE);
					parsedParameters = sscanf (line, "%[^':\n']:%u", fileNameValue, &sizeValue);
	
					// Well parsed!
					if (parsedParameters == 2){
						preloadFile.fileName = fileNameValue;
						preloadFile.sizeKB = sizeValue;
						preloadVector.push_back (preloadFile);
						
					}
					
					else{
						info << "Error parsing preload defined in basic file system: " << line;
						throw cRuntimeError(info.str().c_str());
					}
				}
			}
		}
}
		

int CfgPreloadFS::getNumFiles(){

	return (preloadVector.size());
}


string CfgPreloadFS::getFileName (unsigned int index){

	if (index>=preloadVector.size())
		throw cRuntimeError("Index out of bounds in getFileName (CfgPreloadFS)");
	else
		return preloadVector[index].fileName;
}


unsigned int CfgPreloadFS::getSizeKB (unsigned int index){

	if (index>=preloadVector.size())
		throw cRuntimeError("Index out of bounds in getSizeKB (CfgPreloadFS)");
	else
		return preloadVector[index].sizeKB;
}


string CfgPreloadFS::toString (){
	
	int i;
	std::ostringstream info;

		if (preloadVector.size() == 0)		
			info << "Preload list is empty!" << endl;

		else{
			info << "Printing preload file list from cfgPreloadFS class..." << endl;
			
			for (i=0; i<((int)preloadVector.size()); i++){

				info << "  File [" << i << "]"  
					 << "  Name:" << preloadVector[i].fileName 
					 << "  Size:" << preloadVector[i].sizeKB 
					 << " KB" << endl;
			}
		}
		
	return info.str();
}


vector<string> CfgPreloadFS::divide(const char* inputString){

	vector<string> parts;

	parts.clear();

	std::istringstream f(inputString);

    std::string s;
    while (std::getline(f, s, '$')) {
        parts.push_back(s);
    }

	return parts;

}

} // namespace icancloud
} // namespace inet
