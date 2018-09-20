//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
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

#ifndef __INET_IEEE80211LLCLPD_H
#define __INET_IEEE80211LLCLPD_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211LlcLpd : public Ieee8022Llc, public IIeee80211Llc
{
  protected:
    virtual void encapsulate(Packet *frame) override;

  public:
    const Protocol *getProtocol() const override;
};

} // namespace ieee80211
} // namespace inet

#endif // ifndef __INET_IEEE80211LLCLPD_H

