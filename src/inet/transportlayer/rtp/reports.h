/***************************************************************************
                          reports.h  -  description
                             -------------------
    begin                : Tue Oct 23 2001
    copyright            : (C) 2001 by Matthias Oppitz
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

/** \file reports.h
 * This file declares the classes SenderReport and ReceptionReport as used
 * in RTCPSenderReportPacket and RTCPReceiverReportPacket.
 */

#ifndef __INET_REPORTS_H
#define __INET_REPORTS_H

#include "inet/transportlayer/rtp/reports_m.h"

namespace inet {

namespace rtp {

/**
 * The class SenderReport represents an RTP sender report as contained
 * in an RTCPSenderReportPacket.
 */
class SenderReport : public SenderReport_Base
{
  public:
    SenderReport() : SenderReport_Base() {}
    SenderReport(const SenderReport& other) : SenderReport_Base(other) {}
    SenderReport& operator=(const SenderReport& other) { SenderReport_Base::operator=(other); return *this; }
    virtual SenderReport *dup() const { return new SenderReport(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from SenderReport_Base

  public:
    /**
     * Writes a short info about this SenderReport into the given string.
     */
    virtual std::string info() const;

    /**
     * Writes a longer info about this SenderReport into the given stream.
     */
    virtual void dump(std::ostream& os) const;
};

/**
 * The class ReceptionReport represents an RTP receiver report stored
 * in an RTPSenderReportPacket or RTPReceiverReport.
 */
class ReceptionReport : public ReceptionReport_Base
{
  public:
    ReceptionReport() : ReceptionReport_Base() {}
    ReceptionReport(const ReceptionReport& other) : ReceptionReport_Base(other) {}
    ReceptionReport& operator=(const ReceptionReport& other) { ReceptionReport_Base::operator=(other); return *this; }
    virtual ReceptionReport *dup() const { return new ReceptionReport(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from ReceptionReport_Base

  public:
    /**
     * Writes a short info about this ReceptionReport into the given string.
     */
    virtual std::string info() const;

    /**
     * Writes a longer info about this ReceptionReport into the given stream.
     */
    virtual void dump(std::ostream& os) const;
};

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_REPORTS_H

