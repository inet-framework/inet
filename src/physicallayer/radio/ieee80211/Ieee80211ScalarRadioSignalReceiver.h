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

#ifndef __INET_IEEE80211SCALARRADIOSIGNALRECEIVER_H_
#define __INET_IEEE80211SCALARRADIOSIGNALRECEIVER_H_

#include "ScalarImplementation.h"
#include "WifiPreambleType.h"
#include "IErrorModel.h"
#include "BerParseFile.h"

class INET_API Ieee80211ScalarRadioSignalReceiver : public ScalarRadioSignalReceiver
{
    protected:
        char opMode;
        WifiPreamble preambleMode;
        IErrorModel *errorModel;
        bool autoHeaderSize;
        BerParseFile *parseTable;

    protected:
        virtual void initialize(int stage);

        virtual bool computeHasBitError(double snirMin, int lengthMPDU, double bitrate) const;

    public:
        Ieee80211ScalarRadioSignalReceiver() :
            ScalarRadioSignalReceiver(),
            opMode('\0'),
            preambleMode((WifiPreamble)-1),
            errorModel(NULL),
            autoHeaderSize(false),
            parseTable(NULL)
        {}

        Ieee80211ScalarRadioSignalReceiver(const IModulation *modulation, double snirThreshold, W energyDetecion, W sensitivity, Hz carrierFrequency, Hz bandwidth) :
            ScalarRadioSignalReceiver(modulation, snirThreshold, energyDetecion, sensitivity, carrierFrequency, bandwidth),
            opMode('\0'),
            preambleMode((WifiPreamble)-1),
            errorModel(NULL),
            autoHeaderSize(false),
            parseTable(NULL)
        {}

        virtual ~Ieee80211ScalarRadioSignalReceiver();

        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
};

#endif
