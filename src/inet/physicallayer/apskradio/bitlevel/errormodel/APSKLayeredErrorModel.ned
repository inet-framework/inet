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

package inet.physicallayer.apskradio.bitlevel.errormodel;

import inet.physicallayer.base.bitlevel.LayeredErrorModelBase;

//
// This error model computes the erroneous bits, symbols, or simply whether the
// packet is error free or not based on the SNIR.
//
module APSKLayeredErrorModel extends LayeredErrorModelBase
{
    parameters:
        @class(APSKLayeredErrorModel);
}
