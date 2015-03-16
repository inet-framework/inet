//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.physicallayer.apskradio.packetlevel.errormodel;

import inet.physicallayer.base.packetlevel.ErrorModelBase;

//
// This error model determines packet error rate, bit error rate, and symbol
// error rate by using the well-known formula that corresponds to the modulation.
// It assumes no forward error correction or any other techinque is used in the
// physical signal.
//
module APSKErrorModel extends ErrorModelBase
{
    parameters:
        @class(APSKErrorModel);
}
