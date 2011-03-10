//
//  Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
//  Documentation for these modules is at http://veins.car2x.org/
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package inet.applications.traci;

//
// Processes an XML script to test the OMNeT++ TraCI client modules.
//
// Parameter "commands" takes as input an XML fragment of the following form:
//
// <commands>
//   <CMD_SETMAXSPEED t="10" maxspeed="0" />
//   <CMD_CHANGEROUTE dt="0" roadid="28" traveltime="9999" />
// </commands>
//
// Execution times can be specified relative to the start of the 
// simulation ("t") or the last command ("dt"), i.e. the above example sends 
// CMD_SETMAXSPEED at t=10 and CMD_CHANGEROUTE as soon as the vehicle enters
// the simulation.
//
simple Test_TraCI
{
    parameters:
        bool debug = default(false);  // output debugging information
        xml commands;  // test script to run

        @display("i=block/app2");
    gates:

}
