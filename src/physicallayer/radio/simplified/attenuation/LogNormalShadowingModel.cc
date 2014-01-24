/* **************************************************************************
 * file:        LogNormalShadowing.cc
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
 * description: - This Module implements the log normal shadowing radio propagations model
 *
 ***************************************************************************
 */

#include "LogNormalShadowingModel.h"

Register_Class(LogNormalShadowingModel);
void LogNormalShadowingModel::initializeFrom(cModule *radioModule)
{
    initializeFreeSpace(radioModule);
    sigma = radioModule->par("sigma");
}



double LogNormalShadowingModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    double waveLength = SPEED_OF_LIGHT / carrierFrequency;
    double d0 = 1.0;

    // Reference Pathloss

    double PL_d0 = freeSpace(Gt, Gr, L, pSend, waveLength, d0, pathLossAlpha);
    double PL_d0_db = 10.0 * log10(pSend / PL_d0);

    // Pathloss at distance d + normal distribution
    // normal-distr. assumes std-deviation: s
    double PL_db = PL_d0_db + 10 * pathLossAlpha * log10(distance/d0) + normal(0.0, sigma);

    // Reception power = Tx Power - Pathloss
    double Prx_db = (10.0 * log10(pSend)) - PL_db;

    //EV << "dist " << distance << " avg_power " << f2db(avg->power(distance, Pt)) << " lns_power " << Prx_db << endl;

    // convert dBm to mW
    double prec = pow(10, Prx_db/10.0);
    if (prec > pSend)
        prec = pSend;
    return prec;
}

