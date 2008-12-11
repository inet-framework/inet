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

    cModule *cc = ChannelControl::get();
    if (pathLossAlpha < (double) (cc->par("alpha")))
        opp_error("PathLossReceptionModel: pathLossAlpha can't be smaller than in ChannelControl -- please adjust the parameters");
}

double PathLossReceptionModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    const double speedOfLight = 300000000.0;
    double waveLength = speedOfLight / carrierFrequency;
    return (pSend * waveLength * waveLength / (16 * M_PI * M_PI * pow(distance, pathLossAlpha)));
}

