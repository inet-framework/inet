/* -*- mode:c++ -*- ********************************************************
 * file:        Ieee802154MacLoss.h
 *
 * author:     Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *                Marc Loebbers, Yosia Hadisusanto, Alfonso Ariza
 * copyright:   (C) 2019 Universidad de Málaga
 *              (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *              (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/

#ifndef __INET_IEEE802154MACLOSS_H
#define __INET_IEEE802154MACLOSS_H

#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/contract/IMacProtocol.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/linklayer/ieee802154/Ieee802154Mac.h"

namespace inet {

/**
 * @brief Generic CSMA Mac-Layer.
 *
 * Supports constant, linear and exponential backoffs as well as
 * MAC ACKs.
 *
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 *
 * \image html csmaFSM.png "CSMA Mac-Layer - finite state machine"
 */
class INET_API Ieee802154MacLoss : public Ieee802154Mac
{
  public:
    Ieee802154MacLoss()
        : Ieee802154Mac()
    {}

    virtual ~Ieee802154MacLoss();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;


    virtual void sendUp(cMessage *) override ;



  protected:
    std::map<L3Address, double> probability;

    virtual bool discard(const L3Address &addr);

  private:
    /** @brief Copy constructor is not allowed.
     */
    Ieee802154MacLoss(const Ieee802154MacLoss&);
    /** @brief Assignment operator is not allowed.
     */
    Ieee802154MacLoss& operator=(const Ieee802154MacLoss&);
};

} // namespace inet

#endif // ifndef __INET_IEEE802154MAC_H

