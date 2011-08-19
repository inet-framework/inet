/*
 *  Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  Documentation for these modules is at http://veins.car2x.org/
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

#ifndef TraCITestApp_H
#define TraCITestApp_H

#include <set>
#include <list>

#include <omnetpp.h>

#include "mobility/traci/TraCIMobility.h"

/**
 * FIXME
 */
class TraCITestApp : public cSimpleModule, public INotifiable {
	public:
		int numInitStages() const {return std::max(cSimpleModule::numInitStages(), 1);}
		void initialize(int stage);
		void finish();

	protected:
		// module parameters
		bool debug;
		int testNumber;

		TraCIMobility* traci;
		std::set<std::string> visitedEdges; /**< set of edges this vehicle visited */
		bool hasStopped; /**< true if at some point in time this vehicle travelled at negligible speed */

	protected:
		void handleSelfMsg(cMessage* msg);
		void handleLowerMsg(cMessage* msg);
		void handleMessage(cMessage* msg);

		/**
		 * Callback for NotificationBoard to inform module via NF_HOSTPOSITION_UPDATED
		 */
		void receiveChangeNotification(int category, const cPolymorphic *details);

		void handlePositionUpdate();
};

#endif
