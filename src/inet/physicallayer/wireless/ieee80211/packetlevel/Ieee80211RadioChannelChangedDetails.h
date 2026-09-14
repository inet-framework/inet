//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211RADIOCHANNELCHANGEDDETAILS_H
#define __INET_IEEE80211RADIOCHANNELCHANGEDDETAILS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class IIeee80211Band;

/**
 * Details for radioChannelChanged; the signal value is the band-local channel
 * index. The details object is valid only during notification. The band is
 * immutable, externally owned, and must outlive the radio and its listeners.
 */
class INET_API Ieee80211RadioChannelChangedDetails : public cObject
{
  protected:
    const IIeee80211Band *band;

  public:
    explicit Ieee80211RadioChannelChangedDetails(const IIeee80211Band *band) : band(band) {}
    const IIeee80211Band *getBand() const { return band; }
};

} // namespace physicallayer
} // namespace inet

#endif
