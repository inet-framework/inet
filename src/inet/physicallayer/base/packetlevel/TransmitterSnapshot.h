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

#ifndef __INET_TRANSMITTERSNAPSHOT_H
#define __INET_TRANSMITTERSNAPSHOT_H

#include "inet/physicallayer/contract/packetlevel/IAntennaSnapshot.h"
#include <memory>

namespace inet {

namespace physicallayer {

class IRadio;
class IRadioMedium;

class INET_API TransmitterSnapshot
{
public:
    TransmitterSnapshot(const IRadio* transmitter);

    /**
     * Return the id of the transmitter radio.
     */
    int getId() const;

    /**
     * Return snapshot of transmitter antenna at time of transmission.
     */
    const IAntennaSnapshot *getAntenna() const;

    /**
     * Return radio medium of corresponding transmission.
     */
    const IRadioMedium *getMedium() const;

    /**
     * Try to get transmitter radio if it still exists and is registered
     * at radio medium.
     * Thus, returned pointer may be a nullptr.
     */
    const IRadio *tryRadio() const;

private:
    int transmitterRadioId;
    const IRadioMedium* radioMedium;
    std::shared_ptr<const IAntennaSnapshot> antennaSnapshot;
};

} // namespace physicallayer

} // namespace inet

#endif /* __INET_TRANSMITTERSNAPSHOT_H */

