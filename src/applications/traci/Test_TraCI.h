/*
 *  Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef Test_TraCI_H
#define Test_TraCI_H

#include <set>
#include <list>

#include <omnetpp.h>

#include "mobility/traci/TraCIMobility.h"
#include "world/traci/TraCIScenarioManager.h"

/**
 * Processes an XML script to test the OMNeT++ TraCI client modules.
 *
 * Parameter "commands" takes as input an XML fragment of the following form:
 *
 * <commands>
 *   <CMD_SETMAXSPEED t="10" maxspeed="0" />
 *   <CMD_CHANGEROUTE dt="0" roadid="28" traveltime="9999" />
 * </commands>
 *
 * Execution times can be specified relative to the start of the
 * simulation ("t") or the last command ("dt"), i.e. the above example sends
 * CMD_SETMAXSPEED at t=10 and CMD_CHANGEROUTE as soon as the vehicle enters
 * the simulation.
 *
 * Results are written as scalars and/or appended to Test_TraCI.log
 *
 */
class Test_TraCI : public cSimpleModule, public INotifiable {
	public:

		int numInitStages() const {return 1;}
		virtual void initialize(int);
		virtual void finish();
		virtual void handleMessage(cMessage* msg);

		/**
		 * Callback for NotificationBoard to inform module via NF_HOSTPOSITION_UPDATED
		 */
		virtual void receiveChangeNotification(int category, const cPolymorphic *details);

	protected:
		void executeCommand(const cXMLElement* xmlElement);

		template<typename T> T extract(cDynamicExpression& o) {
			throw std::runtime_error("extract called for unknown type");
		}

		template<typename T> T parseOrBail(const cXMLElement* xmlElement, std::string name) {
			try {
				const char* value_s = xmlElement->getAttribute(name.c_str());
				if (!value_s) throw new std::runtime_error("missing attribute");
				cDynamicExpression value_e;
				value_e.parse(value_s);
				return extract<T>(value_e);
			}
			catch (std::runtime_error e) {
				error((std::string("command parse error for attribute \"") + name + "\": " + e.what()).c_str());
				throw e;
			}
		}

		TraCIMobility* mobility;
		std::list<const cXMLElement*> commands; /**< list of commands to execute, ordered by time */
		static bool clearedLog; /**< true if the logfile has already been cleared */

		// module parameters
		bool debug;

		// statistics output
		std::set<std::string> visitedEdges; /**< set of edges this vehicle visited */
		bool hasStopped; /**< true if at some point in time this vehicle travelled at negligible speed */
};

template<> double Test_TraCI::extract(cDynamicExpression& o);
template<> float Test_TraCI::extract(cDynamicExpression& o);
template<> uint8_t Test_TraCI::extract(cDynamicExpression& o);
template<> std::string Test_TraCI::extract(cDynamicExpression& o);
template<> std::string Test_TraCI::parseOrBail(const cXMLElement* xmlElement, std::string name);

#endif
