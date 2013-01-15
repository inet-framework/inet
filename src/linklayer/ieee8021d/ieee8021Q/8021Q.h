 /**
******************************************************
* @file 8021Q.h
* @brief Relevant definitions for 802.1Q 802.1ad and 802.1ah
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#ifndef __INET_8021Q_H
#define __INET_8021Q_H


typedef unsigned int vid; ///VLAN id type

#define ETHER_1Q_TAG_LENGTH      (4) /* VID(12bit)+DEI(1bit)+PCP(3bit)+TPID(2B) */
#define ETHER_1AH_ITAG_LENGTH		(6) /* I-Tag(6)+B-Tag(4)+B-src(6)+B-dest(6) */
#define ETHER_MAC_2ND_FRAME_BYTES	(12)  /* src(6)+dest(6)    Does not have Length/Type and FCS*/
#define ETHER_II_DISPLAY_STRING		"b=,,,#656665"
#define ETHER_1Q_DISPLAY_STRING		"b=,,,#659965"
#define ETHER_1AD_DISPLAY_STRING	"b=,,,#65cc65"
#define ETHER_1AH_DISPLAY_STRING	"b=,,,#65ff65"
#endif
