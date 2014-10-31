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

#ifndef __INET_IEEE80211ERRORMODELBASE_H
#define __INET_IEEE80211ERRORMODELBASE_H

#include "inet/physicallayer/base/ErrorModelBase.h"
#include "inet/physicallayer/common/ModulationType.h"
#include "inet/physicallayer/ieee80211/BerParseFile.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211ErrorModelBase : public ErrorModelBase
{
  protected:
    // TODO: remove opMode from here and also from BerParseFile
    char opMode;
    bool autoHeaderSize;
    BerParseFile *berTableFile;

  protected:
    virtual void initialize(int stage);
    virtual double GetChunkSuccessRate(ModulationType mode, double snr, uint32_t nbits) const = 0;

  public:
    Ieee80211ErrorModelBase();
    virtual ~Ieee80211ErrorModelBase();

    virtual double computePacketErrorRate(const ISNIR *snir) const;
    virtual double computeBitErrorRate(const ISNIR *snir) const;
    virtual double computeSymbolErrorRate(const ISNIR *snir) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211ERRORMODELBASE_H

