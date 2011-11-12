
// ***************************************************************************
// 
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable 
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ***************************************************************************


#ifndef __httptController_H_
#define __httptController_H_

#include <string>
#include <list>
#include <vector>
#include <omnetpp.h>
#include <fstream>
#include "httptLogdefs.h"
#include "httptUtils.h"
#include "httptRandom.h"
#include "httptEventMessages_m.h"

using namespace std;

// Definitions for the insertion of registered servers into the picklist.
#define INSERT_END 0
#define INSERT_RANDOM -1
#define INSERT_MIDDLE -2

enum ServerStatus {ss_normal, ss_special};

/**
 * @brief Registration entry for Web servers
 */
struct WEB_SERVER_ENTRY
{
	string name;				// The server URL.
	string host;				// The OMNeT++ object name.
	int port;					// Listening port.
	cModule *module;			// The actual OMNeT++ module reference to the server object.
	simtime_t activationTime;	// Activation time for the server relative to simulation start.
	simtime_t statusSetTime;	// Special status set time.
	ServerStatus serverStatus;	// The default is ss_normal -- no modification events set.
	double pvalue;				// Special (elevated) picking probability if ss_special is set.
	double pamortize;			// Amortization factor -- reduces special probability on each hit.
	unsigned long accessCount;	// A counter for the number of server hits.
};

#define STATUS_UPDATE_MSG 0

/**
 * @brief The controller module for HttpTools simulations.
 *
 * A controller object for OMNeT++ simulations which use the HttpTools browser and
 * server components. A single controller must exist at the scenario level in each
 * simulation.
 *
 * @see httptBrowserBase
 * @see httptServerBase
 *
 * @version 1.0
 * @author  Kristjan V. Jonsson 
 */
class httptController : public cSimpleModule
{
	protected:
		int ll; //> The log level

		map<string,WEB_SERVER_ENTRY*> webSiteList; 	//> A list of registered web sites (server objects)
		vector<WEB_SERVER_ENTRY*> pickList;			//> The picklist used to select sites at random.
		list<WEB_SERVER_ENTRY*> specialList;		//> The special list -- contains sites with active popularity modification events.
		double pspecial;							//> The probability [0,1) of selecting a site from the special list.

		unsigned long totalLookups;		//> A counter for the total number of lookups

		rdObject *rdServerSelection;	//> The random object for the server selection.

	/** @name cSimpleModule redefinitions */
	//@{
	protected:
		/** 
		 * @brief Initialization of the component and startup of browse event scheduling. 
		 * Multi-stage is required to properly initialize the object for random site selection after
		 * all servers have been registered.
		 */
		virtual void initialize(int stage);

		/** @brief Report final statistics */
		virtual void finish();

		/** @brief Handle incoming messages */
		virtual void handleMessage(cMessage *msg);

		/** @brief Returns the number of initialization stages. Two required. */
		int numInitStages() const {return 2;}
	//@}

	/** @name public initerface used by server and browser objects in the simulation */
	//@{
	public:
		/**
		 * @brief Register a WWW server object.
		 * Called by server objects at startup. @see httptServerBase.
		 * A datastructure is created for the registered server for easy lookup. It is entered into 
		 * the site picklist which is used to select servers in the general population. The insertion is specified
		 * by the INSERT_* defines: Registered sites can be inserted at the end, in the middle or at random. This does
		 * play a role when zipf or other non-uniform selection probability is used (the pick list is basically mapped to 
		 * the non-uniform distribution).
		 */
		void registerWWWserver( const char* objectName, const char* wwwName, int port, int rank=INSERT_RANDOM, simtime_t activationTime=0.0 );

		/**
		 * @brief Get a specific server module reference
		 * Returns a OMNeT++ module reference for a specific WWW name. Called by browser modules to get a
		 * communications partner. @see httptBrowserBase
		 */
		cModule* getServerModule( const char* wwwName );

		/**
		 * @brief Get a random server object
		 * Returns a OMNeT++ module reference to a randomly chosen server. The general popularity distribution
		 * is used in conjunction with the special list to determine the module returned. Called by browser
		 * modules to get a random communications partner. @see httptBrowserBase
		 */
		cModule* getAnyServerModule();

		/**
		 * @brief Get module and port for a server
		 * Get a module reference and port number for a specific www name. Used by the browser modules
		 * when using TCP transport. @see httptBrowser
		 */
		int getServerInfo( const char* wwwName, char* module, int &port );

		/**
		 * @brief Get module and port for a random server.
		 * Returns a OMNeT++ module name and port number for a randomly chosen server. The general popularity distribution
		 * is used in conjunction with the special list to determine the module returned. Called by browser
		 * modules to get a random communications partner. @see httptBrowserBase
		 */
		int getAnyServerInfo( char* wwwName, char* module, int &port );
	//@}

	protected:
		/** @brief helper used by the server registration to locate the tcpApp getModule(server or browser) */
		cModule* getTcpApp(string node);

		/** @brief Set special status of a WWW server. Triggered by an event message. */
		void setSpecialStatus( const char* www, ServerStatus status, double p, double amortize );

		/** @brief Cancel special popularity status for a server. Called when popularity has been amortized to zero. */
		void cancelSpecialStatus( const char* www );

		/** @brief Select a server from the special list. This method is called with the pspecial probability. */
		WEB_SERVER_ENTRY* selectFromSpecialList();

		/** @brief list the registered servers. Useful for debug. */
		string listRegisteredServers();

		/** @brief list the servers on the special list, i.e. those with custom selection probability. Useful for debug. */
		string listSpecials();

		/** @brief list the registered servers in the order of the general population pick list. Useful for debug. */
		string listPickOrder();

		/** 
		 * @brief Parse a popularity modification events definition file at startup (if defined) 
	 	 * Format: {time};{www name};{event kind};{p value};{amortization factor}
		 * Event kind is not used at the present -- use 1 as a default here.
		*/
		void parseOptionsFile(string file, string section);

	private:
		/** @brief Get a random server from the special list with p=pspecial or from the general population with p=1-pspecial. */
		WEB_SERVER_ENTRY* __getRandomServerInfo();
};

#endif /* httptController */


