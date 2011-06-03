/* **************************************************************************
 * file:        Rayleigh.cc
 *
 * author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
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
 * part of:     SNE4OMF
 * description: - This Module implements the rayleigh radio propagations model
 * http://www.tm.uka.de/sne4omf
 *
 ***************************************************************************
 */
#include "RayleighModel.h"
#include <math.h>
#include <FWMath.h>

Register_Class(RayleighModel);
void RayleighModel::initializeFrom(cModule *radioModule)
{
    initializeFreeSpace(radioModule);
}

RayleighModel::~RayleighModel() {

}


double RayleighModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    double waveLength = SPEED_OF_LIGHT / carrierFrequency;
    double avg_rx_power = freeSpace(Gt, Gr, L, pSend, waveLength, distance, pathLossAlpha);

    double x = normal(0, 1);
    double y = normal(0, 1);
    double prec = avg_rx_power * 0.5 * (x*x + y*y);
    if (prec > pSend)
        prec = pSend;
    return prec;

}

