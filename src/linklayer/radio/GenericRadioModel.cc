//
// Copyright (C) 2006 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "GenericRadioModel.h"
#include "Modulation.h"
#include "FWMath.h"


Register_Class(GenericRadioModel);

GenericRadioModel::GenericRadioModel()
{
    modulation = NULL;
}

GenericRadioModel::~GenericRadioModel()
{
    delete modulation;
}

void GenericRadioModel::initializeFrom(cModule *radioModule)
{
    snirThreshold = dB2fraction(radioModule->par("snirThreshold"));
    headerLengthBits = radioModule->par("headerLengthBits");
    bandwidth = radioModule->par("bandwidth");

    const char *modulationName = radioModule->par("modulation");
    if (strcmp(modulationName, "null")==0)
        modulation = new NullModulation();
    else if (strcmp(modulationName, "BPSK")==0)
        modulation = new BPSKModulation();
    else if (strcmp(modulationName, "16-QAM")==0)
        modulation = new QAM16Modulation();
    else if (strcmp(modulationName, "256-QAM")==0)
        modulation = new QAM256Modulation();
    else
        throw cRuntimeError(this, "unrecognized modulation '%s'", modulationName);
}


double GenericRadioModel::calculateDuration(AirFrame *airframe)
{
    return (airframe->getBitLength()+headerLengthBits) / airframe->getBitrate();
}


bool GenericRadioModel::isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList)
{
    // calculate snirMin
    double snirMin = receivedList.begin()->snr;
    for (SnrList::const_iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;

    if (snirMin <= snirThreshold)
    {
        // if snir is too low for the packet to be recognized
        EV << "COLLISION! Packet got lost\n";
        return false;
    }
    else if (isPacketOK(snirMin, airframe->getBitLength()+headerLengthBits, airframe->getBitrate()))
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


bool GenericRadioModel::isPacketOK(double snirMin, int length, double bitrate)
{
    double ber = modulation->calculateBER(snirMin, bandwidth, bitrate);

    if (ber==0.0)
        return true;

    double probNoError = pow(1.0 - ber, length); // probability of no bit error

    if (dblrand() > probNoError)
        return false; // error in MPDU
    else
        return true; // no error
}

double GenericRadioModel::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}


