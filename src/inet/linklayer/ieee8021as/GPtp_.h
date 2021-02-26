//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef GPTP_H_
#define GPTP_H_

namespace inet {

/* Below is used to calculate packet transmission time */
enum gPtpHeader: int {
    MAC_HEADER = 22,
    CRC_CHECKSUM = 4
};

}

#endif /* GPTP_H_ */
