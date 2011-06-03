/* **************************************************************************
 * file:        Nakagami.cc
 *
 * author:      Andreas Kuntz
 *
 * copyright:   (c) 2009 Institute of Telematics, University of Karlsruhe (TH)
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
 * description: - This Module implements the nakagami radio propagations model
 *
 ***************************************************************************
 */

#include "NakagamiModel.h"

Register_Class(NakagamiModel);

void NakagamiModel::initializeFrom(cModule *radioModule)
{
    initializeFreeSpace(radioModule);
    m = radioModule->par("nak_m");
}


double NakagamiModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    const int rng = 0;
    double waveLength = SPEED_OF_LIGHT / carrierFrequency;

    double avg_power = freeSpace(Gt, Gr, L, pSend, waveLength, distance, pathLossAlpha);
    avg_power = avg_power/1000;
    double prec = gamma_d(m, avg_power / m, rng) * 1000.0;
     if (prec > pSend)
        prec = pSend;
     return prec;
}
