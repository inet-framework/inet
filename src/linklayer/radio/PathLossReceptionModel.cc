//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "PathLossReceptionModel.h"
#include "ChannelControl.h"
#include "FWMath.h"

Register_Class(PathLossReceptionModel);

void PathLossReceptionModel::initializeFrom(cModule *radioModule)
{
    pathLossAlpha = radioModule->par("pathLossAlpha");
    shadowingDeviation = radioModule->par("shadowingDeviation");

    cModule *cc = ChannelControl::get();
    if (pathLossAlpha < (double) (cc->par("alpha")))
        opp_error("PathLossReceptionModel: pathLossAlpha can't be smaller than in ChannelControl -- please adjust the parameters");
}

double PathLossReceptionModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    const double speedOfLight = 300000000.0;
    double waveLength = speedOfLight / carrierFrequency;
    if (shadowingDeviation == 0.0)
        return pSend * waveLength * waveLength / (16 * M_PI * M_PI * pow(distance, pathLossAlpha));
    else
    {
        // This code implements a shadowing component for the path loss reception model. The random
        // variable has a normal distribution in dB and results to log-normal distribution in mW.
        // This is a widespread and common model used for reproducing shadowing effects
        // (Rappaport, T. S. (2002), Wireless Communications - Principles and Practice, Prentice Hall PTR).
        double xs = normal(0.0, shadowingDeviation);
        double mWValue = pSend * waveLength * waveLength / (16 * M_PI * M_PI * pow(distance, pathLossAlpha));
        double dBmValue = mW2dBm(mWValue);
        dBmValue += xs;
        double mWValueWithShadowing = pow(10.0, dBmValue/10.0);
        return mWValueWithShadowing;
    }
}

double PathLossReceptionModel::mW2dBm(double mW)
{
     return 10*log10(mW);
}
