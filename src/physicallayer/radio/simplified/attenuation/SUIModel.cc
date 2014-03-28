/* **************************************************************************
 * file:        SUIModel.h
 *
 * author:      Konrad Polys, Krzysztof Grochla
 *
 * copyright:   (c) 2013 The Institute of Theoretical and Applied Informatics
 *                       of the Polish Academy of Sciences, Project
                          LIDER/10/194/L-3/11/ supported by NCBIR
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * description: - This Module implements the Stanford University Interim
 *                propagation model.
 *
 ***************************************************************************
 */

#include "SUIModel.h"
#include <math.h>
#include <string>

Register_Class(SUIModel);
void SUIModel::initializeFrom(cModule *radioModule)
{
    initializeFreeSpace(radioModule);
    terrain = radioModule->par("terrain").stringValue();
    ht = radioModule->par("TransmiterAntennaHigh");
    hr = radioModule->par("ReceiverAntennaHigh");

    Gt = pow(10, radioModule->par("TransmissionAntennaGainIndB").doubleValue()/10);
    Gr = pow(10, radioModule->par("ReceiveAntennaGainIndB").doubleValue()/10);

}

double SUIModel::calculateReceivedPower(double pSend, double carrierFrequency, double distance)
{
    /*
     * Terrain A - Highest path loss. Dense populated urban area.
     * Terrain B - Intermediate path loss. Suburban area.
     * Terrain C - Minimum path loss. Flat areas or rural with light vegetation.
     */

    double a,b,c,d,s;
    if (terrain=="TerrainA") { a=4.6;   b=0.0075;   c=12.6; d=10.8; s=10.6; }
    if (terrain=="TerrainB") { a=4.0;   b=0.0065;   c=17.1; d=10.8; s=9.6;  }
    if (terrain=="TerrainC") { a=3.6;   b=0.0050;   c=20.0; d=20.0; s=8.2;  }

    double R = distance;    // [m]
    double R0 = 100.0;      // [m]
    double lambda = SPEED_OF_LIGHT / carrierFrequency;
    double Pr = 0.0;        // [dBm]
    double Pt = 10*log10(pSend/1);  // [dBm]
    double prec = 0.0;     // [mW]
    double L = 0.0;        // [dBm]
    double f = carrierFrequency / 1000000000.0; // [GHz]

    double alpha = 0.0;
    double gamma = a - b*ht + c/ht;
    double Xf = 6 * log10( f/2 );
    double Xh = -d * log10( hr/2 );

    double R0p = R0 * pow(10.0,-( (Xf+Xh) / (10*gamma) ));


    if(R>R0p)
    {
        alpha = 20 * log10( (4*M_PI*R0p) / lambda );
        L = alpha + 10*gamma*log10( R/R0 ) + Xf + Xh + s;
    }
    else
    {
        L = 20 * log10( (4*M_PI*R) / lambda ) + s;
    }

    Pr = Pt + Gt + Gr - L;

    prec = pow(10, Pr/10.0); // [dBm]->[mW]


    if (prec > pSend)
        prec = pSend;
    return prec;

}



