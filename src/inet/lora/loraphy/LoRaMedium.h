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
#ifndef LORAPHY_LORAMEDIUM_H_
#define LORAPHY_LORAMEDIUM_H_
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/common/IntervalTree.h"
#include "inet/environment/contract/IMaterialRegistry.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/physicallayer/common/packetlevel/CommunicationLog.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/contract/packetlevel/ICommunicationCache.h"
#include "inet/physicallayer/contract/packetlevel/IMediumLimitCache.h"
#include "inet/physicallayer/contract/packetlevel/INeighborCache.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include <algorithm>

#include "LoRaRadio.h"
namespace inet {
namespace physicallayer {
class INET_API LoRaMedium : public RadioMedium
{
    friend class LoRaGWRadio;
    friend class LoRaRadio;

protected:
    virtual bool matchesMacAddressFilter(const IRadio *radio, const Packet *packet) const override;
        //@}
    public:
      LoRaMedium();
      virtual ~LoRaMedium();
      virtual const IReceptionDecision *getReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const override;
      virtual const IReceptionResult *getReceptionResult(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const override;
};
}
}
#endif /* LORAPHY_LORAMEDIUM_H_ */
