//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package inet.tests.traci;

import inet.world.ChannelControl;
import inet.world.traci.TraCIScenarioManagerLaunchd;

module Highway
{
    parameters:
        double playgroundSizeX;
        double playgroundSizeY;
    submodules:
        channelcontrol: ChannelControl {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=4360,3657;i=map");
        }
        manager: TraCIScenarioManagerLaunchd {
            parameters:
                @display("p=140,50;i=block/cogwheel_s");
        }
    connections allowunconnected:
}

network highway1 extends Highway
{
    parameters:
}

