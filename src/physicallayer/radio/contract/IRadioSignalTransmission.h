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

#ifndef __INET_IRADIOSIGNALTRANSMISSION_H_
#define __INET_IRADIOSIGNALTRANSMISSION_H_

#include "Units.h"
#include "Coord.h"
#include "IPrintableObject.h"

class IRadio;

/**
 * This interface represents the transmission of a radio signal.
 */
class INET_API IRadioSignalTransmission : public IPrintableObject
{
    public:
        virtual ~IRadioSignalTransmission() {}

        /**
         * Returns an identifier for this transmission which is globally unique
         * for the whole lifetime of the simulation among all transmissions.
         */
        virtual int getId() const = 0;

        /**
         * Returns the transmitter that transmitted this radio signal on the radio
         * channel. This function never returns NULL.
         */
        virtual const IRadio *getTransmitter() const = 0;

        /**
         * TODO
         */
        virtual const cPacket *getMacFrame() const = 0;

        /**
         * Returns the time when the transmitter started this transmission. It is
         * the start of the first bit's transmission.
         */
        virtual const simtime_t getStartTime() const = 0;

        /**
         * Returns the time when the transmitter ended this transmission. It is
         * the end of the last bit's transmission.
         */
        virtual const simtime_t getEndTime() const = 0;

        /**
         * Returns the position where the transmitter started this transmission.
         */
        virtual const Coord getStartPosition() const = 0;

        /**
         * Returns the position where the transmitter ended this transmission.
         */
        // TODO: FIXME: we don't know the end position at the time of the transmission begin, we can only have a guess
        // TODO: should we separate transmission begin and end? or should it be an approximation only?
        virtual const Coord getEndPosition() const = 0;

        // TODO: use the propagation model instead?
        virtual mps getPropagationSpeed() const = 0;
};

#endif
