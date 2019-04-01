#ifndef __CFG_PRELOAD_FS_
#define __CFG_PRELOAD_FS_

#include <omnetpp.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "inet/icancloud/Base/include/icancloud_types.h"

namespace inet {

namespace icancloud {


using std::string;
using std::vector;


/*
 * @class cfgPreloadFS cfgPreloadFS.h "cfgPreloadFS.h"
 *
 * Module to contains all files to be preloaded at system and defined by tenant when a concrete application is executed
 *
 * @author Gabriel Gonzalez Castane
 * @date 2013-05-04
 */


class CfgPreloadFS{

	struct cfgPreload_t{
			string fileName;			
			unsigned int sizeKB;
	};
	typedef struct cfgPreload_t preload_T;


	public:

		std::vector <preload_T> preloadVector;

		~CfgPreloadFS();
		CfgPreloadFS ();

		void parseFile (const char* fileName);
		int getNumFiles ();
		string getFileName (unsigned int index);
		unsigned int getSizeKB (unsigned int index);		
		string toString ();
		vector<string> divide(const char* inputString);
};

} // namespace icancloud
} // namespace inet

#endif
