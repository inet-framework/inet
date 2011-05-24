cplusplus {{
#include "Ieee802154Const.h"
#include "Ieee802154Enum.h"
}}


enum PHYenum;

enum PHYPIBenum;


message Ieee802154MacPhyPrimitives
{
    short status enum(PHYenum);
    short attribute enum(PHYPIBenum);
    double   bitRate;
    short additional=-1; 

    unsigned char energyLevel;

    int channelNumber; 		// the current radio channel
		//unsigned int	phyChannelsSupported; 
		//double		phyTransmitPower;
		//unsigned char	phyCCAMode;
}
