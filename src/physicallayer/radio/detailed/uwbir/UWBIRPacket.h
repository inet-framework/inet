/* -*- mode:c++ -*- ********************************************************
 * file:        Packet.h
 *
 * author:      Jérôme Rousselot
 *
 * copyright:   (C) 2009 CSEM SA
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#ifndef BBUWBIRPACKET_H
#define BBUWBIRPACKET_H

#include <sstream>
#include <string>
#include <omnetpp.h>

#include "MiXiMDefs.h"

/**
 * @brief Class that keeps track of the number of packets sent.
 *
 * @author Andreas Koepke
 */
class MIXIM_API  UWBIRPacket : public cObject
{
protected:
    /** @brief number of packets generated. */
    long nbSyncAttempts;
    long nbSyncSuccesses;
    long nbPacketsReceived;

public:

    /** @brief Constructor*/
    UWBIRPacket() : cObject(), nbSyncAttempts(0), nbSyncSuccesses(0), nbPacketsReceived(0) { };

    long getNbPacketsReceived() const  {
        return nbPacketsReceived;
    }

    void setNbPacketsReceived(int n)  {
        nbPacketsReceived = n;
    }

    long getNbSyncAttempts() const  {
            return nbSyncAttempts;
	}

    void setNbSyncAttempts(long n)  {
            nbSyncAttempts = n;
	}

    long getNbSyncSuccesses() const {
    	return nbSyncSuccesses;
    }

    void setNbSyncSuccesses(long n) {
    	nbSyncSuccesses = n;
    }

    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " Number of packets generated is "<< 0;
        return ost.str();
    }
};


#endif
