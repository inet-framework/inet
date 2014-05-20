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

#ifndef __INET_IPHYSICALLAYERCONTROLINFO_H_
#define __INET_IPHYSICALLAYERCONTROLINFO_H_

#include "PhysicalLayerDefs.h"

/**
 * This purely virtual interface provides an abstraction for different physical layer indications.
 */
class INET_API IPhysicalLayerControlInfo
{
    public:
        virtual ~IPhysicalLayerControlInfo() { }

        /**
         * Returns true if the packet is actually free of errors.
         */
        virtual bool isPacketErrorless() const = 0;

        /**
         * Returns the actual number of erroneous bits or -1 if unknown.
         */
        virtual int getBitErrorCount() const = 0;

        /**
         * Returns the actual number of erroneous symbols or -1 if unknown.
         */
        virtual int getSymbolErrorCount() const = 0;

        /**
         * Returns the packet error rate (probability) or NaN if unknown.
         */
        virtual double getPER() const = 0;

        /**
         * Returns the bit error rate (probability) or NaN if unknown.
         */
        virtual double getBER() const = 0;

        /**
         * Returns the symbol error rate (probability) or NaN if unknown.
         */
        virtual double getSER() const = 0;

        /**
         * Returns the receive signal strength indication or NaN if unknown.
         */
        virtual const W getRSSI() const = 0;

        /**
         * Returns the signal to noise plus interference ratio or NaN if unknown.
         */
        virtual double getSNIR() const = 0;
};

#endif
