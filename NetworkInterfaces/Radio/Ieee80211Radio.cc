//
// Copyright (C) Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//FIXME docu

#include "Ieee80211Radio.h"
#include "TransmComplete_m.h"
#include "FWMath.h"
#include "Consts80211.h"  //XXX for 802.11 only



#define TRANSM_OVER 1  // timer to indicate that a message is completely sent now


Define_Module(Ieee80211Radio);

void Ieee80211Radio::initialize(int stage)
{
    AbstractRadio::initialize(stage);
}

/**
 * Usually the duration is just the frame length divided by the
 * bitrate. However there may be cases (like 802.11) where the header
 * has a different modulation (and thus a different bitrate) than the
 * rest of the message.
 *
 * XXX Ieee80211:
 * The header is sent with 1Mbit/s and the rest with the bitrate read in in initialize().
 */
double Ieee80211Radio::calcDuration(AirFrame *af)
{
    //XXX generic:
    //double duration;
    //duration = (double) af->length() / (double) bitrate;
    //return duration;

    //XXX Ieee80211:
    EV << "bits without header: " << af->length() -
        headerLength << ", bits header: " << headerLength << endl;
    return ((af->length() - headerLength) / bitrate + headerLength / BITRATE_HEADER);
}

/**
 * This function simply calculates with how much power the signal
 * arrives "here". If a different way of computing the path loss is
 * required this function can be redefined.
 */
double Ieee80211Radio::calcRcvdPower(double pSend, double distance)
{
    double speedOfLight = 300000000.0;
    double waveLength = speedOfLight / carrierFrequency;
    return (pSend * waveLength * waveLength / (16 * M_PI * M_PI * pow(distance, pathLossAlpha)));
}

