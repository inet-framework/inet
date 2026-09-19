//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_IIEEE80211TRANSMITTERCAPABILITIES_H
#define INET_IIEEE80211TRANSMITTERCAPABILITIES_H

#include "inet/common/Units.h"

namespace inet::physicallayer {

/** Read-only implemented HT abilities, ready after physical-layer initialization.
 * False means unsupported, independently of the selected BSS operation.
 */
class INET_API IIeee80211TransmitterCapabilities
{
  public:
    virtual ~IIeee80211TransmitterCapabilities() = default;
    [[nodiscard]] virtual bool isHtChannelWidthSupported(Hz channelWidth) const = 0;
};

} // namespace inet::physicallayer

#endif
