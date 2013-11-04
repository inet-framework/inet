/* -*- mode:c++ -*-  *******************************************************
 * file:        SnrList.h
 *
 * author:      Marc Löbbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
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

#ifndef SNRLIST_H
#define SNRLIST_H

#include <list>

/**
 * @brief struct for SNR information
 *
 * used to store SNR information of a message and pass it to the
 * Decider. Each SnrListEntry corresponds to one SNR value at a
 * specific time.
 *
 * @ingroup basicUtils
 * @ingroup utils
 * @author Marc Löbbers
 */
struct SnrListEntry{
  /** @brief timestamp for this SNR value*/
  simtime_t time;
  /** @brief the SNR value*/
  double snr;
};

/**
 * @brief List to store SNR information for a message
 *
 * used to store SNR information of a message and pass it to the
 * Decider. Each SnrListEntry in this list corresponds to one SNR
 * value at a specific time.
 *
 * @ingroup utils
 * @ingroup basicUtils
 * @author Marc Löbbers
 */
typedef std::list<SnrListEntry> SnrList;

#endif
