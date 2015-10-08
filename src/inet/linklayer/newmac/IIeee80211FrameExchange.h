//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef IIEEE80211MACFRAMEEXCHANGE_H_
#define IIEEE80211MACFRAMEEXCHANGE_H_

#include "Ieee80211MacPlugin.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;

class IIeee80211FrameExchange
{
    public:
        class IFinishedCallback {
            public:
                virtual void frameExchangeFinished(IIeee80211FrameExchange *what, bool successful) = 0;
                virtual ~IFinishedCallback() {}
        };

    public:
        virtual void start() = 0;
        virtual bool lowerFrameReceived(Ieee80211Frame *frame) = 0;  // true = processed
        virtual void transmissionFinished() = 0;
};

}
} /* namespace inet */

#endif

