//
// TraCIMobility - Mobility module to be controlled by TraCIScenarioManager
// Copyright (C) 2006 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
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

package inet.mobility.traci;

//
// TraCIMobility is a mobility module for hosts controlled by TraCIScenarioManager.
// It receives position and state updates from an external module and updates
// the parent module accordingly.
//
simple TraCIMobility
{
    parameters:
        @display("i=block/cogwheel");
        bool debug = default(false);  // debug switch
        int accidentCount = default(0);  // number of accidents
        double accidentStart @unit("s") = default(uniform(30s,60s));  // time until first accident, relative to departure time
        volatile double accidentDuration @unit("s") = default(uniform(30s,60s));  // duration of accident
        volatile double accidentInterval @unit("s") = default(uniform(30s,60s));  // time between accidents
}

