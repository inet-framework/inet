//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.wireless.neuralnetwork;

import inet.physicallayer.contract.packetlevel.IRadio;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.physicallayer.errormodel.packetlevel.Ieee80211RadioErrorModelEvaluator;

network EvaluateIeee80211RadioErrorModels
{
    submodules:
        radioMedium: <> like IRadioMedium {
            parameters:
                @display("p=200,100");
        }
        radio: <> like IRadio {
            parameters:
                @display("p=200,200");
        }
        evaluator: Ieee80211RadioErrorModelEvaluator {
            parameters:
                @display("p=200,300");
        }
    connections allowunconnected:
}
