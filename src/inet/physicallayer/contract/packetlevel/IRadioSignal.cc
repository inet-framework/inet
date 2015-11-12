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

#include "inet/physicallayer/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

cEnum *IRadioSignal::signalPartEnum = nullptr;

Register_Enum(inet::physicallayer::IRadioSignal::SignalPart,
    (IRadioSignal::SIGNAL_PART_NONE,
     IRadioSignal::SIGNAL_PART_WHOLE,
     IRadioSignal::SIGNAL_PART_PREAMBLE,
     IRadioSignal::SIGNAL_PART_HEADER,
     IRadioSignal::SIGNAL_PART_DATA));

const char *IRadioSignal::getSignalPartName(SignalPart signalPart)
{
    if (signalPartEnum == nullptr)
        signalPartEnum = cEnum::get(opp_typename(typeid(IRadioSignal::SignalPart)));
    return signalPartEnum->getStringFor(signalPart) + 12;
}

} // namespace physicallayer

} // namespace inet

