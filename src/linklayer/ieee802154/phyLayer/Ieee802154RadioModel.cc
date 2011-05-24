
#include "Ieee802154RadioModel.h"
#include "Ieee802154Const.h"
#include "FWMath.h"

Register_Class(Ieee802154RadioModel);
static const double BER_LOWER_BOUND = 1e-10;

void Ieee802154RadioModel::initializeFrom(cModule *radioModule)
{
    // read from Ieee802154phy
    snirThreshold = dB2fraction(radioModule->par("snirThreshold"));
    ownerRadioModule = radioModule;
}

double Ieee802154RadioModel::calculateDuration(AirFrame *airframe)
{
    return (def_phyHeaderLength*8 + airframe->getBitLength())/airframe->getBitrate() ;
}


bool Ieee802154RadioModel::isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList)
{
    // calculate snirMin
    double snirMin = receivedList.begin()->snr;
    for (SnrList::const_iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;
#if OMNETPP_VERSION>0x0400
    cPacket *frame = airframe->getEncapsulatedPacket();
# else
    cPacket *frame = airframe->getEncapsulatedMsg();
#endif
    EV << "packet (" << frame->getClassName() << ")" << frame->getName() << " (" << frame->info() << ") snrMin=" << snirMin << endl;

    if (snirMin <= snirThreshold)
    {
        // if snir is too low for the packet to be recognized
        EV << "COLLISION! Packet got lost\n";
        return false;
    }
    if (!packetOk(snirMin, airframe->getEncapsulatedMsg()->getBitLength(), airframe->getBitrate()))
    {
    	EV << "Packet has BIT ERRORS! It is lost!\n";
    	return false;
    }
    /*else if (packetOk(snirMin, airframe->getEncapsulatedMsg()->length(), airframe->getBitrate()))
    {
        EV << "packet was received correctly, it is now handed to upper layer...\n";
        return true;
    }
    else
    {
        EV << "Packet has BIT ERRORS! It is lost!\n";
        return false;
    }*/

    return true;
}



bool Ieee802154RadioModel::packetOk(double snirMin, int lengthMPDU, double bitrate)
{

	if (ownerRadioModule->par("NoBitError"))
			return true;

	double errorHeader;

    double  ber = std::max(0.5 * exp(-snirMin /2), BER_LOWER_BOUND);
    errorHeader = 1.0 - pow((1.0 - ber), def_phyHeaderLength*8);

    double MpduError = 1.0 - pow((1.0 - ber), lengthMPDU);

    EV << "ber: " << ber << endl;
    double rand = dblrand();

    if (dblrand() < errorHeader)
        return false; // error in header
    else if (dblrand() < MpduError)
        return false;  // error in MPDU
    else
        return true; // no error
}



double Ieee802154RadioModel::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}

