//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_IIEEE80211RECEIVERCAPABILITIES_H
#define INET_IIEEE80211RECEIVERCAPABILITIES_H

#include "inet/common/Units.h"

namespace inet::physicallayer {

/** Read-only implemented HT abilities, ready after physical-layer initialization.
 * False means unsupported, independently of the selected BSS operation.
 */
class INET_API IIeee80211ReceiverCapabilities
{
  public:
    virtual ~IIeee80211ReceiverCapabilities() = default;
    [[nodiscard]] virtual bool isHtChannelWidthSupported(Hz channelWidth) const = 0;
    [[nodiscard]] virtual bool isHtShortGuardIntervalSupported(Hz channelWidth) const = 0;
};

} // namespace inet::physicallayer

#endif
