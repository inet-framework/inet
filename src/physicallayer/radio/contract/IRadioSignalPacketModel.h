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

#ifndef __INET_IRADIOSIGNALPACKETMODEL_H_
#define __INET_IRADIOSIGNALPACKETMODEL_H_

#include "IPrintableObject.h"

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the packet domain.
 */
class INET_API IRadioSignalPacketModel : public IPrintableObject
{
    public:
        virtual ~IRadioSignalPacketModel() {}

        virtual const cPacket *getPacket() const = 0;
};

class INET_API IRadioSignalTransmissionPacketModel : public virtual IRadioSignalPacketModel
{
};

class INET_API IRadioSignalReceptionPacketModel : public virtual IRadioSignalPacketModel
{
    public:
        /**
         * Returns the packet error rate (probability).
         */
        virtual double getPER() const = 0;

        /**
         * Returns true if the packet is actually free of errors.
         */
        virtual bool isPacketErrorless() const = 0;
};

#endif
