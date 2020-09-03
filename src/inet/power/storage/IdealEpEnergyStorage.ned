//
// Copyright (C) OpenSim Ltd.
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

package inet.power.storage;

import inet.power.base.EpEnergyStorageBase;

//
// This energy storage model stores an infinite amount of energy. It can provide
// energy for any number of consumers, and it can absorb energy from any number
// of generators. The ideal energy storage never gets completely charged or
// depleted. This module is primarily useful for testing energy consumer and
// energy generator models. See the base module for signals and statistics.
//
//
simple IdealEpEnergyStorage extends EpEnergyStorageBase
{
    parameters:
        @class(IdealEpEnergyStorage);
}

