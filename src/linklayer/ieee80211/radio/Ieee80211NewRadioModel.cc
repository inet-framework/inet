//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
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
// 2010 Alfoso Ariza (universidad de Málaga), new radio model, inspired in the yans and ns3 models

#include "Ieee80211NewRadioModel.h"
#include "Ieee80211Consts.h"
#include "FWMath.h"
#define NS3CALMODE


Register_Class(Ieee80211NewRadioModel);

Ieee80211NewRadioModel::~Ieee80211NewRadioModel()
{
    if (parseTable)
        delete parseTable;
}


void Ieee80211NewRadioModel::initializeFrom(cModule *radioModule)
{
    snirThreshold = dB2fraction(radioModule->par("snirThreshold"));


    if (strcmp("SHORT", radioModule->par("WifiPreambleMode").stringValue())==0)
        wifiPreamble = WIFI_PREAMBLE_SHORT;
    else if (strcmp("LONG", radioModule->par("WifiPreambleMode").stringValue())==0)
        wifiPreamble = WIFI_PREAMBLE_LONG;
    else
        wifiPreamble = WIFI_PREAMBLE_LONG;

    if (strcmp("b", radioModule->par("phyOpMode").stringValue())==0)
        phyOpMode = 'b';
    else if (strcmp("g", radioModule->par("phyOpMode").stringValue())==0)
        phyOpMode = 'g';
    else if (strcmp("a", radioModule->par("phyOpMode").stringValue())==0)
        phyOpMode = 'a';
    else if (strcmp("p", radioModule->par("phyOpMode").stringValue())==0)
        phyOpMode = 'p';
    else
        phyOpMode = 'g';

    autoHeaderSize = radioModule->par("AutoHeaderSize");

    parseTable = NULL;
    PHY_HEADER_LENGTH = 26e-6;

    snirVector.setName("snirVector");
    i = 0;
    const char *fname = radioModule->par("berTableFile");
    std::string name(fname);
    if (!name.empty())
    {
        parseTable = new BerParseFile(phyOpMode);
        parseTable->parseFile(fname);
        fileBer = true;
    }
    else
        fileBer = false;
}



double Ieee80211NewRadioModel::calculateDuration(AirFrame *airframe)
{
    double duration;
#ifndef NS3CALMODE
    if (phyOpMode=='g')
        duration = 4*ceil((16+airframe->getBitLength()+6)/(airframe->getBitrate()/1e6*4))*1e-6 + PHY_HEADER_LENGTH;
    else
        // The physical layer header is sent with 1Mbit/s and the rest with the frame's bitrate
        duration = airframe->getBitLength()/airframe->getBitrate() + 192/BITRATE_HEADER;
#else
    ModulationType modeBody;
    if (phyOpMode=='g')
    {
        modeBody = WifyModulationType::getMode80211g(airframe->getBitrate());
        duration = SIMTIME_DBL(WifyModulationType::calculateTxDuration(airframe->getBitLength(), modeBody, wifiPreamble));
    }
    else if (phyOpMode=='b')
    {
        // The physical layer header is sent with 1Mbit/s and the rest with the frame's bitrate
        modeBody = WifyModulationType::getMode80211b(airframe->getBitrate());
        duration = SIMTIME_DBL(WifyModulationType::calculateTxDuration(airframe->getBitLength(), modeBody, wifiPreamble));
    }
    else if (phyOpMode=='a')
    {
        // The physical layer header is sent with 1Mbit/s and the rest with the frame's bitrate
        modeBody = WifyModulationType::getMode80211a(airframe->getBitrate());
        duration = SIMTIME_DBL(WifyModulationType::calculateTxDuration(airframe->getBitLength(), modeBody, wifiPreamble));
    }
    else if (phyOpMode=='p')
    {
        // The physical layer header is sent with 1Mbit/s and the rest with the frame's bitrate
        modeBody = WifyModulationType::getMode80211p(airframe->getBitrate());
        duration = SIMTIME_DBL(WifyModulationType::calculateTxDuration(airframe->getBitLength(), modeBody, wifiPreamble));
    }
    else
        opp_error("Radio model not supported yet, must be a,b,g or p");
    airframe->setModulationType(modeBody);
#endif
    EV<<"Radio:frameDuration="<<duration*1e6<<"us("<<airframe->getBitLength()<<"bits)"<<endl;
    return duration;
}


bool Ieee80211NewRadioModel::isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList)
{
    // calculate snirMin
    double snirMin = receivedList.begin()->snr;
    for (SnrList::const_iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;

    cPacket *frame = airframe->getEncapsulatedPacket();
    EV << "packet (" << frame->getClassName() << ")" << frame->getName() << " (" << frame->info() << ") snrMin=" << snirMin << endl;

    if (i%1000==0)
    {
        snirVector.record(10*log10(snirMin));
        i = 0;
    }
    i++;

    if (snirMin <= snirThreshold)
    {
        // if snir is too low for the packet to be recognized
        EV << "COLLISION! Packet got lost. Noise only\n";
        return false;
    }
    else if (isPacketOK(snirMin, frame->getBitLength(), airframe->getBitrate()))
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


bool Ieee80211NewRadioModel::isPacketOK(double snirMin, int lengthMPDU, double bitrate)
{
    double berHeader, berMPDU;
    ModulationType modeBody;
    ModulationType modeHeader;

    WifiPreamble preambleUsed = wifiPreamble;
    double headerNoError;
    uint32_t headerSize;
    if (phyOpMode=='b')
        headerSize = HEADER_WITHOUT_PREAMBLE;
    else
        headerSize = 24;

    if (phyOpMode=='g')
    {
        modeBody = WifyModulationType::getMode80211g(bitrate);
        modeHeader = WifyModulationType::getPlcpHeaderMode(modeBody, preambleUsed);
        if (autoHeaderSize)
        {
           ModulationType modeBodyA = WifyModulationType::getMode80211a(bitrate);
           headerSize = ceil(SIMTIME_DBL(WifyModulationType::getPlcpHeaderDuration(modeBodyA, preambleUsed))*modeHeader.getDataRate());
        }
    }
    else if (phyOpMode=='b')
    {
        modeBody = WifyModulationType::getMode80211b(bitrate);
        modeHeader = WifyModulationType::getPlcpHeaderMode(modeBody, preambleUsed);
        if (autoHeaderSize)
            headerSize = ceil(SIMTIME_DBL(WifyModulationType::getPlcpHeaderDuration(modeBody, preambleUsed))*modeHeader.getDataRate());
    }
    else if (phyOpMode=='a')
    {
        modeBody = WifyModulationType::getMode80211a(bitrate);
        modeHeader = WifyModulationType::getPlcpHeaderMode(modeBody, preambleUsed);
        if (autoHeaderSize)
             headerSize = ceil(SIMTIME_DBL(WifyModulationType::getPlcpHeaderDuration(modeBody, preambleUsed))*modeHeader.getDataRate());
    }
    else if (phyOpMode=='p')
    {
        modeBody = WifyModulationType::getMode80211p(bitrate);
        modeHeader = WifyModulationType::getPlcpHeaderMode(modeBody, preambleUsed);
        if (autoHeaderSize)
             headerSize = ceil(SIMTIME_DBL(WifyModulationType::getPlcpHeaderDuration(modeBody, preambleUsed))*modeHeader.getDataRate());
    }
    else
    {
        opp_error("Radio model not supported yet, must be a,b,g or p");
    }

    headerNoError = yansModel.GetChunkSuccessRate(modeHeader, snirMin, headerSize);
    // probability of no bit error in the MPDU
    double MpduNoError;
    if (fileBer)
        MpduNoError = 1-parseTable->getPer(bitrate, snirMin, lengthMPDU/8);
    else
        MpduNoError = yansModel.GetChunkSuccessRate(modeHeader, snirMin, lengthMPDU);

    EV << "berHeader: " << berHeader << " berMPDU: " <<berMPDU <<" lengthMPDU: "<<lengthMPDU<<" PER: "<<1-MpduNoError<<endl;
    if (MpduNoError>=1 && headerNoError>=1)
        return true;
    double rand = dblrand();

    if (rand > headerNoError)
        return false; // error in header
    else if (dblrand() > MpduNoError)
        return false;  // error in MPDU
    else
        return true; // no error
}

double Ieee80211NewRadioModel::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}

