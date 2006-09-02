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


#include "Ieee80211Radio.h"
#include "TransmComplete_m.h"
#include "FWMath.h"
#include "Consts80211.h"  //XXX for 802.11 only



Define_Module(Ieee80211Radio);

void Ieee80211Radio::initialize(int stage)
{
    AbstractRadio::initialize(stage);

    if (stage==0)
    {
        snirThreshold = dB2fraction(par("snirThreshold")); //XXX use different name then...
    }
}

/**
 * The header is sent with 1Mbit/s and the rest with "bitrate"
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


bool Ieee80211Radio::isReceivedCorrectly(AirFrame *af, const SnrList& receivedList)
{
    // calculate snirMin
    double snirMin = receivedList.begin()->snr;
    for (SnrList::const_iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;

    cMessage *fr = af->encapsulatedMsg();
    EV << "packet (" << fr->className() << ")" << fr->name() << " (" << fr->info() << ") snrMin=" << snirMin << endl;

    if (snirMin <= snirThreshold)
    {
        // if snir is too low for the packet to be recognized
        EV << "COLLISION! Packet got lost\n";
        return false;
    }
    else if (packetOk(snirMin, af->encapsulatedMsg()->length()))
    {
        EV << "packet was received correctly, it is now handed to upper layer...\n";
        return true;
    }
    else
    {
        EV << "Packet has BIT ERRORS! It is lost!\n";
        return false;
    }
}


bool Ieee80211Radio::packetOk(double snirMin, int lengthMPDU)
{
    double berHeader, berMPDU;

    berHeader = 0.5 * exp(-snirMin * BANDWIDTH / BITRATE_HEADER);
    //if PSK modulation
    if (bitrate == 1E+6 || bitrate == 2E+6)
        berMPDU = 0.5 * exp(-snirMin * BANDWIDTH / bitrate);
    //if CCK modulation (modeled with 16-QAM)
    else if (bitrate == 5.5E+6)
        berMPDU = 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snirMin * BANDWIDTH / bitrate);
    else                        // CCK, modelled with 256-QAM
        berMPDU = 0.25 * (1 - 1 / sqrt(pow(2.0, 8))) * erfc(snirMin * BANDWIDTH / bitrate);
    //probability of no bit error in the PLCP header
    double headerNoError = pow(1.0 - berHeader, HEADER_WITHOUT_PREAMBLE);

    //probability of no bit error in the MPDU
    double MpduNoError = pow(1.0 - berMPDU, lengthMPDU);
    EV << "berHeader: " << berHeader << " berMPDU: " << berMPDU << endl;
    double rand = dblrand();

    //if error in header
    if (rand > headerNoError)
        return (false);
    else
    {
        rand = dblrand();

        //if error in MPDU
        if (rand > MpduNoError)
            return (false);
        //if no error
        else
            return (true);
    }
}

double Ieee80211Radio::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}


