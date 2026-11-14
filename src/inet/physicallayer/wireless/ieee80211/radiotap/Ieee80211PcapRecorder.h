// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211PCAPRECORDER_H
#define INET_IEEE80211PCAPRECORDER_H

#include "inet/common/packet/recorder/PcapRecorder.h"

namespace inet::physicallayer {

/** Records each validated-layout A-MPDU member through the ordinary recorder.
 * This observation adapter does not select production decoding or responses.
 */
class INET_API Ieee80211PcapRecorder : public PcapRecorder
{
  protected:
    uint32_t nextAmpduReference = 0;
    void recordPacket(const cPacket *packet, Direction direction, cComponent *source) override;
};

} // namespace inet::physicallayer
#endif
