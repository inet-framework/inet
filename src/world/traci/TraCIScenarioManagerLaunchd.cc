//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "world/traci/TraCIScenarioManagerLaunchd.h"
#include "world/traci/TraCIConstants.h"
#define CMD_FILE_SEND 0x75

#include <sstream>
#include <iostream>
#include <fstream>

#define MYDEBUG EV

Define_Module(TraCIScenarioManagerLaunchd);

TraCIScenarioManagerLaunchd::~TraCIScenarioManagerLaunchd()
{
}

void TraCIScenarioManagerLaunchd::initialize(int stage)
{
    //TODO why call the base initialize() at the end?

    if (stage == 0)
    {
        launchConfig = par("launchConfig").xmlValue();
        seed = par("seed");
    }
    else if (stage == 1)
    {
        cXMLElementList basedir_nodes = launchConfig->getElementsByTagName("basedir");
        if (basedir_nodes.size() == 0) {
            // default basedir is where current network file was loaded from
            std::string basedir = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
            cXMLElement* basedir_node = new cXMLElement("basedir", __FILE__, launchConfig);
            basedir_node->setAttribute("path", basedir.c_str());
            launchConfig->appendChild(basedir_node);
        }
        cXMLElementList seed_nodes = launchConfig->getElementsByTagName("seed");
        if (seed_nodes.size() == 0) {
            if (seed == -1) {
                // default seed is current repetition
                const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
                seed = atoi(seed_s);
            }
            std::stringstream ss; ss << seed;
            cXMLElement* seed_node = new cXMLElement("seed", __FILE__, launchConfig);
            seed_node->setAttribute("value", ss.str().c_str());
            launchConfig->appendChild(seed_node);
        }
    }

    TraCIScenarioManager::initialize(stage);
}

void TraCIScenarioManagerLaunchd::finish()
{
    TraCIScenarioManager::finish();
}

void TraCIScenarioManagerLaunchd::init_traci() {
    {
        std::pair<uint32_t, std::string> version = TraCIScenarioManager::commandGetVersion();
        uint32_t apiVersion = version.first;
        std::string serverVersion = version.second;

        ASSERT(apiVersion == 1);

        EV_DEBUG << "TraCI launchd reports version \"" << serverVersion << "\"" << endl;
    }

    std::string contents = launchConfig->tostr(0);

    TraCIBuffer buf;
    buf << std::string("sumo-launchd.launch.xml") << contents;
    sendTraCIMessage(makeTraCICommand(CMD_FILE_SEND, buf));

    TraCIBuffer obuf(receiveTraCIMessage());
    uint8_t cmdLength; obuf >> cmdLength;
    uint8_t commandResp; obuf >> commandResp; if (commandResp != CMD_FILE_SEND) error("Expected response to command %d, but got one for command %d", CMD_FILE_SEND, commandResp);
    uint8_t result; obuf >> result;
    std::string description; obuf >> description;
    if (result != RTYPE_OK) {
        EV << "Warning: Received non-OK response from TraCI server to command " << CMD_FILE_SEND << ":" << description.c_str() << std::endl;
    }

    TraCIScenarioManager::init_traci();
}


