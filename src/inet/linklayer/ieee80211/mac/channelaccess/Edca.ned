//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// 

package inet.linklayer.ieee80211.mac.channelaccess;

import inet.linklayer.ieee80211.mac.contention.Contention;
import inet.linklayer.ieee80211.mac.contract.ICollisionController;


module Edca
{
    parameters:
        int numEdcafs;
        string rxModule;
        string rateSelectionModule;

        *.rateSelectionModule = "^." + rateSelectionModule;
        *.rxModule = "^." + rxModule;
        *.collisionControllerModule = "^.collisionController";

        edcaf[0].accessCategory = "AC_BK";
        edcaf[1].accessCategory = "AC_BE";
        edcaf[2].accessCategory = "AC_VI";
        edcaf[3].accessCategory = "AC_VO";

        @class("Edca");
        @display("i=block/star");
    submodules:
        edcaf[numEdcafs]: Edcaf {
            parameters:
	            @display("p=100,100,row,200");
        }
        collisionController: <default("EdcaCollisionController")> like ICollisionController {
			parameters:
	            @display("p=100,200");
        }
}
