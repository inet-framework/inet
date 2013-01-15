/**
******************************************************
* @file PortFilt1ah.h
* @brief Port associated functions. Filtering, tagging, etc.
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
 * Port associated functions. Filtering, tagging, etc.
 */
class PortFilt1ah : public PortFilt
{
	public:
	PortFilt1ah();
	virtual ~PortFilt1ah();
	protected:

    virtual void initialize();

    /**
     * Filtering and relaying
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Returns Edge parameter.
     */
    virtual bool isEdge();
};

#endif


