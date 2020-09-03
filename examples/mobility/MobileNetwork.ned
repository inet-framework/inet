//
// Copyright (C) 2005 OpenSim Ltd.
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.examples.mobility;

import inet.visualizer.integrated.IntegratedCanvasVisualizer;

network MobileNetwork
{
    parameters:
        int numHosts;
        bool hasVisualizer = default(false);
        @display("bgb=600,400");
    submodules:
        visualizer: IntegratedCanvasVisualizer if hasVisualizer {
            parameters:
                @display("p=100,100");
        }
        host[numHosts]: MobileHost {
            parameters:
                @display("p=300,200;r=,,#707070");
        }
}
