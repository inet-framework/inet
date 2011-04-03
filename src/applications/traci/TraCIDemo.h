/*
 *  Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

#ifndef TraCIDemo_H
#define TraCIDemo_H

#include <omnetpp.h>

#include "mobility/traci/TraCIMobility.h"

/**
 * Small IVC Demo
 */
class TraCIDemo : public BasicModule {
	public:
		virtual int numInitStages() const {
			return std::max(4, BasicModule::numInitStages());
		}
		virtual void initialize(int);
		virtual void receiveChangeNotification(int category, const cPolymorphic* details);
		virtual void handleMessage(cMessage* msg);

	protected:
		bool debug;
		TraCIMobility* traci;
		bool sentMessage;

	protected:
		void setupLowerLayer();
		virtual void handleSelfMsg(cMessage* apMsg);
		virtual void handleLowerMsg(cMessage* apMsg);

		virtual void sendMessage();
		virtual void handlePositionUpdate();
};

#endif

