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

#ifndef __INET_IRADIOCHANNEL_H_
#define __INET_IRADIOCHANNEL_H_

#include "IRadio.h"
#include "IRadioFrame.h"

/**
 * This purely virtual interface provides an abstraction for different radio channels.
 */
class INET_API OldIRadioChannel
{
    public:
        virtual ~OldIRadioChannel() { }

        /**
         * Returns the number of available radio channels.
         */
        virtual int getNumChannels() = 0;
};

#endif
