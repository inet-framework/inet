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

#ifndef __INET_IEEE80211SCALARRECEIVER_H
#define __INET_IEEE80211SCALARRECEIVER_H

#include "ScalarReceiver.h"
#include "WifiPreambleType.h"
#include "IErrorModel.h"
#include "BerParseFile.h"

namespace inet {
namespace physicallayer {
class INET_API Ieee80211ScalarReceiver : public ScalarReceiver
{
  protected:
    char opMode;
    WifiPreamble preambleMode;
    IErrorModel *errorModel;
    bool autoHeaderSize;
    BerParseFile *parseTable;

  protected:
    virtual void initialize(int stage);

    virtual bool computeHasBitError(const IListening *listening, double minSNIR, int bitLength, double bitrate) const;

  public:
    Ieee80211ScalarReceiver() :
        ScalarReceiver(),
        opMode('\0'),
        preambleMode((WifiPreamble) - 1),
        errorModel(NULL),
        autoHeaderSize(false),
        parseTable(NULL)
    {}

    virtual ~Ieee80211ScalarReceiver();
};
} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IEEE80211SCALARRECEIVER_H

