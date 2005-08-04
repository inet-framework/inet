/***************************************************************************
 * file:        Decider80211.cc
 *
 * authors:     David Raguin / Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/


#include "Mac80211Pkt_m.h"
#include "Decider80211.h"
#include "Consts80211.h"


Define_Module(Decider80211);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicDecider.
 *
 * This decider also needs the bitrate and some 802.11 parameters are
 * initialized
 */
void Decider80211::initialize(int stage)
{
    BasicDecider::initialize(stage);

    if (stage == 0)
    {
        EV << "initializing stage 0\n";
        bitrate = par("bitrate");
        if (bitrate != 1E+6 && bitrate != 2E+6 && bitrate != 5.5E+6 && bitrate != 11E+6)
            error("Wrong bitrate!! Please chose 1E+6, 2E+6, 5.5E+6 or 11E+6 as bitrate!!");
        snirThreshold = dB2fraction(par("snirThreshold"));
    }
}


/**
 * Handle message from lower layer. The minimal snir is read in and
 * it is computed wether the packet has collided or has bit errors or
 * was received correctly. The corresponding kind is set and it is
 * handed on to the upper layer.
 *
 */
void Decider80211::handleLowerMsg(AirFrame *af, SnrList& receivedList)
{

    double snirMin;

    //initialize snirMin:
    snirMin = receivedList.begin()->snr;

    for (SnrList::iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
    {
        if (iter->snr < snirMin)
            snirMin = iter->snr;
    }
    EV << "packet from: " << ((Mac80211Pkt *) (af->encapsulatedMsg()))->
        getSrcAddr() << " snrMin: " << snirMin << endl;

    //if snir is big enough so that packet can be recognized at all
    if (snirMin > snirThreshold)
    {
        if (packetOk(snirMin, af->encapsulatedMsg()->length()))
        {
            EV << "packet was received correctly, it is now handed to upper layer...\n";
            sendUp(af);
        }
        else
        {
            EV << "Packet has BIT ERRORS! It is lost!\n";
            af->setName("ERROR");
            af->encapsulatedMsg()->setKind(BITERROR);
            sendUp(af);
        }
    }
    else
    {
        EV << "COLLISION! Packet got lost\n";
        af->setName("COLLISION");
        af->encapsulatedMsg()->setKind(COLLISION);
        sendUp(af);
    }
}


bool Decider80211::packetOk(double snirMin, int lengthMPDU)
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

double Decider80211::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}


#ifdef _WIN32
double Decider80211::erfc(double x)
{
    double t, u, y;

    if (x <= -6)
        return 2;
    if (x >= 6)
        return 0;

    t = 3.97886080735226 / (fabs(x) + 3.97886080735226);
    u = t - 0.5;
    y = (((((((((0.00127109764952614092 * u + 1.19314022838340944e-4) * u -
        0.003963850973605135) * u - 8.70779635317295828e-4) * u +
        0.00773672528313526668) * u + 0.00383335126264887303) * u -
        0.0127223813782122755) * u - 0.0133823644533460069) * u +
        0.0161315329733252248) * u + 0.0390976845588484035) * u +
        0.00249367200053503304;
    y = ((((((((((((y * u - 0.0838864557023001992) * u -
        0.119463959964325415) * u + 0.0166207924969367356) * u +
        0.357524274449531043) * u + 0.805276408752910567) * u +
        1.18902982909273333) * u + 1.37040217682338167) * u +
        1.31314653831023098) * u + 1.07925515155856677) * u +
        0.774368199119538609) * u + 0.490165080585318424) * u +
        0.275374741597376782) * t * exp(-x * x);

    return x < 0 ? 2 - y : y;
}
#endif
