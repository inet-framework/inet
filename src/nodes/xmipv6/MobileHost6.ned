//


package inet.nodes.xmipv6;

import inet.nodes.ipv6.StandardHost6;


//
// \MIPv6 host with TCP, UDP layers and applications.
//
module MobileHost6 extends StandardHost6
{
    networkLayer.xMIPv6Support = true;
    networkLayer.mipv6support.isMobileNode = true; //is Mobile Node
    networkLayer.mipv6support.isHomeAgent = false; //is Home Agent
}

