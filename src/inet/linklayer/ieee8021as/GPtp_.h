//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef __INET_GPTP__H
#define __INET_GPTP__H

namespace inet {

/* Below is used to calculate packet transmission time */
enum gPtpHeader: int {
    MAC_HEADER = 22,
    CRC_CHECKSUM = 4
};

}

#endif

