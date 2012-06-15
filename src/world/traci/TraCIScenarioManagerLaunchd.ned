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

package inet.world.traci;

//
// Extends the TraCIScenarioManager for use with sumo-launchd.py and SUMO.
//
// Connects to a running instance of the sumo-launchd.py script
// to automatically launch/kill SUMO when the simulation starts/ends.
//
// All other functionality is provided by the TraCIScenarioManager.
//
// See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
//
// @author Christoph Sommer, David Eckhoff
//
// @see TraCIMobility
// @see TraCIScenarioManager
//
simple TraCIScenarioManagerLaunchd
{
    parameters:
        @display("i=block/network2");
        bool debug = default(false);  // emit debug messages?
        double connectAt @unit("s") = default(0s);  // when to connect to TraCI server (must be the initial timestep of the server)
        double firstStepAt @unit("s") = default(-1s);  // when to start synchronizing with the TraCI server (-1: immediately after connecting)
        double updateInterval @unit("s") = default(1s);  // time interval of hosts' position updates
        string moduleType = default("inet.nodes.wireless.WirelessHostSimplified");  // module type to be used in the simulation for each managed vehicle
        string moduleName = default("host");  // module name to be used in the simulation for each managed vehicle
        string moduleDisplayString = default("i=misc/node2;is=vs;r=0,,#707070,1");  // module displayString to be used in the simulation for each managed vehicle
        string host = default("localhost");  // sumo-launchd.py server hostname
        int port = default(9999);  // sumo-launchd.py server port
        xml launchConfig; // launch configuration to send to sumo-launchd.py
        int seed = default(-1); // seed value to set in launch configuration, if missing (-1: current run number)
        bool autoShutdown = default(true);  // Shutdown module as soon as no more vehicles are in the simulation
        int margin = default(25);  // margin to add to all received vehicle positions
        string roiRoads = default("");  // which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty
        string roiRects = default("");  // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty
        double penetrationRate = default(1); //the probability of a vehicle being equipped with Car2X technology
}

