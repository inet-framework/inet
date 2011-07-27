#ifndef IEEE_802154_RADIOMODEL_H
#define IEEE_802154_RADIOMODEL_H

#include "INETDefs.h"

#include "IRadioModel.h"
#include "SnrList.h"


class INET_API Ieee802154RadioModel : public IRadioModel
{
  protected:
    double snirThreshold;
    cModule *ownerRadioModule;

  public:
    virtual void initializeFrom(cModule *radioModule);

    virtual double calculateDuration(AirFrame *airframe);

    virtual bool isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList);

  protected:
    // utility
    virtual bool packetOk(double snirMin, int lengthMPDU, double bitrate);
    // utility
    virtual double dB2fraction(double dB);
};

#endif
