/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   Modified by Weverton Cordeiro                                         *
 *   (C) 2007 wevertoncordeiro@gmail.com                                   *
 *   Adapted for omnetpp                                                   *
 *   2008 Alfonso Ariza Quintana aarizaq@uma.es                            *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*** New File Added ***/

#ifndef __OLSR_ETX_parameter_h__
#define __OLSR_ETX_parameter_h__

#include <omnetpp.h>

#define OLSR_ETX_CAPPROBE_PACKET_SIZE 1000
#define CAPPROBE_MAX_ARRAY              16

class OLSR_ETX_parameter : public cObject
{
    /// Determine which MPR selection algorithm to use
#define OLSR_ETX_DEFAULT_MPR         1
#define OLSR_ETX_MPR_R1              2
#define OLSR_ETX_MPR_R2              3
#define OLSR_ETX_MPR_QOLSR           4
#define OLSR_ETX_MPR_OLSRD           5
    int    mpr_algorithm_;
    /// Determine which routing algorith is to be used
#define OLSR_ETX_DEFAULT_ALGORITHM   1
#define OLSR_ETX_DIJKSTRA_ALGORITHM  2
    int    routing_algorithm_;
    /// Determines which heuristic should be used for link quality computation
#define OLSR_ETX_BEHAVIOR_NONE       1
#define OLSR_ETX_BEHAVIOR_ETX        2
#define OLSR_ETX_BEHAVIOR_ML         3
    int    link_quality_;
    /// Determine whether fish eye routing algorithm should be used
    int    fish_eye_;
    /// Determine the redundancy level of TC messages
    // publish only nodes in mpr set
#define OLSR_ETX_TC_REDUNDANCY_MPR_SET       0x1
    // publish only nodes in mpr sel set
#define OLSR_ETX_TC_REDUNDANCY_MPR_SEL_SET   0x2

#define OLSR_ETX_TC_MPR_SEL_SET_REDUNDANCY   0
#define OLSR_ETX_TC_MPR_PLUS_MPR_SEL_SET_REDUNDANCY   1
#define OLSR_ETX_TC_ALL_NEIGHBOR_SET_REDUNDANCY   2
    int    tc_redundancy_;

    /// Link quality extension
    int      link_delay_; // Should link delay estimation be used
    double   c_alpha_; // Factor that will be used to smooth link delays

  public:
    inline OLSR_ETX_parameter()
    {
        //
    }

    inline int&     mpr_algorithm() { return mpr_algorithm_; }
    inline int&     routing_algorithm() { return routing_algorithm_; }
    inline int&     link_quality() { return link_quality_; }
    inline int&     fish_eye() { return fish_eye_; }
    inline int&     tc_redundancy() { return tc_redundancy_; }

    /// Link quality extension
    inline int&     link_delay() { return link_delay_; }
    inline double&  c_alpha() { return c_alpha_; }

};

#endif
