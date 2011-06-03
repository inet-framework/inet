/* **************************************************************************
 * file:        Rice.cc
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
 * description: - This Module implements the rice radio propagations model
 * http://www.tm.uka.de/sne4omf
 *
 ***************************************************************************
 */


#include "RiceModel.h"
#include <math.h>

Register_Class(RiceModel);
void RiceModel::initializeFrom(cModule *radioModule)
{
    initializeFreeSpace(radioModule);
    K = pow(10, radioModule->par("K").doubleValue()/10);
}

double RiceModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    double waveLength = SPEED_OF_LIGHT / carrierFrequency;
    double c = 1.0/(2.0*(K+1));
    double x = normal(0, 1);
    double y = normal(0, 1);
    double rr = c*( (x + sqrt(2*K))*(x + sqrt(2*K)) + y*y);
    double prec = freeSpace(Gt, Gr, L, pSend, waveLength, distance, pathLossAlpha) * rr;
    if (prec > pSend)
        prec = pSend;
    return prec;


}



