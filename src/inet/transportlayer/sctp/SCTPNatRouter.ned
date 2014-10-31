//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.transportlayer.sctp;

import inet.node.inet.StandardHost;


//
// \IP router.
//
module SCTPNatRouter extends StandardHost
{
    parameters:
        forwarding = true;
        natHook.networkProtocolModule = default(absPath(".networkLayer.ip"));
    submodules:
        natHook: SCTPNatHook {
            parameters:
                @display( "p=160,132;i=block/table");
        }
        natTable: SCTPNatTable {
            parameters:
                @display( "p=60,132;i=block/table");
        }
    connections:
        natHook.out --> networkLayer.transportIn++;
        natHook.in <-- networkLayer.transportOut++;
}
