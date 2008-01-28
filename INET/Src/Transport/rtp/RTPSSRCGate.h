/***************************************************************************
                          RTPSSRCGate.h  -  description
                             -------------------
    begin                : Tue Jan 1 2002
    copyright            : (C) 2002 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/** \file RTPSSRCGate.h
 * This file declares the class RTPSSRGate.
 */

#ifndef __RTPSSRCGATE_H__
#define __RTPSSRCGATE_H__


#include <omnetpp.h>
#include "INETDefs.h"
#include "types.h"



/**
 * The class RTPSSRCGate is just a small helper class used by an RTPProfile
 * module to store the association between an ssrc identifier and the gate
 * which leads to the RTPPayloadReceiver module.
 */
class INET_API RTPSSRCGate : public cObject
{

    public:

        /**
         * Default constructor.
         */
        RTPSSRCGate(u_int ssrc = 0);

        /**
         * Copy constructor.
         */
        RTPSSRCGate(const RTPSSRCGate& rtpSSRCGate);

        /**
         * Destructor.
         */
        virtual ~RTPSSRCGate();

        /**
         * Returns the ssrc identifier.
         */
        virtual u_int32 ssrc();

        /**
         * Sets the ssrc identifier.
         */
        virtual void setSSRC(u_int32 ssrc);

        /**
         * Returns the id of the gate.
         */
        virtual int gateId();

        /**
         * Sets the id of the gate.
         */
        virtual void setGateId(int gateId);

    protected:
        /**
         * The ssrc identifier.
         */
        u_int32 _ssrc;

        /**
         * The gate id.
         */
        int _gateId;
};

#endif

