//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef WORLD_TRACI_TRACISCENARIOMANAGERLAUNCHD_H
#define WORLD_TRACI_TRACISCENARIOMANAGERLAUNCHD_H

#include <omnetpp.h>

#include "INETDefs.h"
#include "world/traci/TraCIScenarioManager.h"

/**
 * A TraCIScenarioManager that interacts with sumo-launchd
 *
 * @author Christoph Sommer
 */
class INET_API TraCIScenarioManagerLaunchd : public TraCIScenarioManager
{
	public:

		virtual ~TraCIScenarioManagerLaunchd();
		virtual int numInitStages() const { return std::max(TraCIScenarioManager::numInitStages(), 2); }
		virtual void initialize(int stage);
		virtual void finish();

	protected:

		cXMLElement* launchConfig; /**< launch configuration to send to sumo-launchd */
		int seed; /**< seed value to set in launch configuration, if missing (-1: current run number) */

		virtual void init_traci();
};

class TraCIScenarioManagerLaunchdAccess : public ModuleAccess<TraCIScenarioManagerLaunchd>
{
	public:
		TraCIScenarioManagerLaunchdAccess() : ModuleAccess<TraCIScenarioManagerLaunchd>("manager") {};
};

#endif
