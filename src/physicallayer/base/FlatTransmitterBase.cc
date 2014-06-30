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

#include "FlatTransmitterBase.h"
#include "Modulation.h"

using namespace inet;

using namespace physicallayer;

void FlatTransmitterBase::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        headerBitLength = par("headerBitLength");
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        const char *modulationName = par("modulation");
        if (strcmp(modulationName, "NULL")==0)
            modulation = new NullModulation();
        else if (strcmp(modulationName, "BPSK")==0)
            modulation = new BPSKModulation();
        else if (strcmp(modulationName, "16-QAM")==0)
            modulation = new QAM16Modulation();
        else if (strcmp(modulationName, "256-QAM")==0)
            modulation = new QAM256Modulation();
        else
            throw cRuntimeError(this, "Unknown modulation '%s'", modulationName);
        bitrate = bps(par("bitrate"));
        power = W(par("power"));
    }
}
