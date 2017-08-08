//
// Copyright (C) 2017 Raphael Riebl, TH Ingolstadt
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

#include "inet/physicallayer/base/packetlevel/TransmitterSnapshot.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

TransmitterSnapshot::TransmitterSnapshot(const IRadio* transmitter) :
    transmitterRadioId(transmitter->getId()),
    radioMedium(transmitter->getMedium()),
    antennaSnapshot(transmitter->getAntenna()->getSnapshot())
{
}

int TransmitterSnapshot::getId() const
{
    return transmitterRadioId;
}

const IAntennaSnapshot *TransmitterSnapshot::getAntenna() const
{
    return antennaSnapshot.get();
}

const IRadioMedium *TransmitterSnapshot::getMedium() const
{
    return radioMedium;
}

const IRadio *TransmitterSnapshot::tryRadio() const
{
    return radioMedium->getRadio(transmitterRadioId);
}

} // namespace physicallayer

} // namespace inet

