/* **************************************************************************
 * file:        TwoRayGround.cc
 *
 * author:      Andreas Kuntz
 *
 * copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
 *
 * author:      Alfonso Ariza
 *              Malaga university
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     SNE4OMF   http://www.tm.uka.de/sne4omf
 * description: - This Module implements the two ray ground radio propagations model
 *
 ***************************************************************************
 */
#include "TwoRayGroundModel.h"

Register_Class(TwoRayGroundModel);

void TwoRayGroundModel::initializeFrom(cModule *radioModule)
{
    initializeFreeSpace(radioModule);
    ht = radioModule->par("TransmiterAntennaHigh");
    hr = radioModule->par("ReceiverAntennaHigh");
}


TwoRayGroundModel::~TwoRayGroundModel()
{
}

double TwoRayGroundModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    double waveLength = SPEED_OF_LIGHT / carrierFrequency;

    if (distance == 0)
        return pSend;

    /**
     * cross over distance dc
     * at distance dc the two ray model and free space model predict the same power
     *
     *          4 * pi * hr * ht
     *   dc = ----------------------
     *           lambda
     **/

    double dc = (4 * M_PI * ht * hr ) / waveLength;

    if (distance < dc )
    {
        /**
         * Friis free space equation:
         *
         *       Pt * Gt * Gr * (lambda^2)
         *   P = --------------------------
         *       (4 * pi)^2 * d^2 * L
         */
        return freeSpace(Gt, Gr, L, pSend, waveLength, distance, pathLossAlpha);
    }
    else
    {
        /**
         *  Two-ray ground reflection model.
         *
         *       Pt * gt * gr * (ht^2 * hr^2)
         *  Pr = ----------------------------
         *                 d^4 * L
         *
         * To be consistant with the free space equation, L is added here.
         * The original equation in Rappaport's book assumes L = 1.
         */
        double prec = ((pSend * Gt * Gr * (ht * ht * hr * hr) ) / (distance * distance * distance * distance * L));
        if (prec > pSend)
            prec = pSend;
        return prec;
    }
}
