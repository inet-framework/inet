//
// Copyright (C) 2015 Andras Varga
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#ifndef __INET_IMACRADIOINTERFACE_H
#define __INET_IMACRADIOINTERFACE_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;

/**
 * Rx and Tx processes use this interface to transmit frames over the radio.
 */
class INET_API IMacRadioInterface
{
    public:
        virtual void sendFrame(Ieee80211Frame *frame) = 0;
        virtual void sendDownPendingRadioConfigMsg() = 0;  //TODO remove
};

} // namespace ieee80211
} // namespace inet

#endif

