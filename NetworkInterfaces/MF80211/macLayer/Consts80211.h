#ifndef CONSTANTS_802_11
#define CONSTANTS_802_11

//frame kinds
enum _802_11frameType {

  //between MAC layers of two nodes
  DATA = 1,//data
  BROADCAST = 2,//broadcast
  RTS = 3,//request to send
  CTS = 4,//clear to send
  ACK = 5,//acknowledgement
  ACKRTS = 6,//cut through packet

  //between the PHY and the MAC layer of one node
  BEGIN_RECEPTION = 7,//carrier sensing from the phy to the mac :
  //beginning of reception

  //used in the phy layer to indicate unrocognizable frames, and
  //between the decider and the MAC
  BITERROR = -1,//the phy has recognized a bit error in the packet
  COLLISION = 9//packet lost due to collision
};

//frame lengths in bits
const unsigned int LENGTH_RTS = 160;
const unsigned int LENGTH_CTS = 112;
const unsigned int LENGTH_ACK = 112;

//time slot ST, short interframe space SIFS, distributed interframe
//space DIFS, and extended interframe space EIFS
const double ST = 20E-6;
const double SIFS = 10E-6;
const double DIFS = 2*ST + SIFS;

const int RETRY_LIMIT = 7;

/**Minimum size (initial size) of contention window*/
const int CW_MIN = 7;

/** Maximum size of contention window*/
const int CW_MAX = 255;

const int PHY_HEADER_LENGTH=192;
const int HEADER_WITHOUT_PREAMBLE=48;
const double BITRATE_HEADER=1E+6;
const double BANDWIDTH=2E+6;


const int MAC_GENERATOR = 5;

const double PROCESSING_TIMEOUT = 0.001;

#endif
