//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "Ieee80211RadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211RadioMedium);

void Ieee80211RadioMedium::initialize(int stage)
{
    RadioMedium::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numChannels = par("numChannels");
    }
}

} // namespace physicallayer

} // namespace inet

