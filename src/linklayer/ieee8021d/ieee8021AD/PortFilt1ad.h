 /**
******************************************************
* @file PortFilt1ad.h
* @brief Tagging and filtering skills.
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __INET_PortFilt1AD_H
#define __INET_PortFilt1AD_H

#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "8021Q.h"
#include "PortFilt.h"


/**
 * Tagging skills
 */
class PortFilt1ad : public PortFilt
{
  protected:
    std::vector <vid> registeredCVids;

  public:
    PortFilt1ad();
    ~PortFilt1ad();
  protected:
    virtual void initialize(); // Base class initialization. Adding CVIDs reading.
    virtual void handleMessage(cMessage *msg);
    virtual void sendMVRPDUs();


    /**
     * @brief Reads itagtable info from configIS xml.
     */
	virtual void readCVIDsfromXML(const cXMLElement* CVIDstab);

	/**
	 * @brief Tagging if necessary.
	 */
    virtual void processUntaggedFrame(EthernetIIFrame *msg);
};

#endif


