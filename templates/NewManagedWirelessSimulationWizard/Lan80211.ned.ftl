<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">
package ${nedPackageName};
</#if>

// numOfHosts: ${numOfHosts}

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.wireless.WirelessAPSimplified;
import inet.nodes.wireless.WirelessHostSimplified;
import inet.world.ChannelControl;


network ${targetTypeName}
{
    parameters:
        int numOfHosts;
        int playgroundSizeX;
        int playgroundSizeY;

    submodules:
        host[numOfHosts]: WirelessHostSimplified
        {
            @display("r=,,#707070");
        }

        ap: WirelessAPSimplified
        {
            @display("p=213,174;r=,,#707070");
        }

        channelcontrol: ChannelControl
        {
            playgroundSizeX = playgroundSizeX;
            playgroundSizeY = playgroundSizeY;
            numChannels = 2;
            @display("p=61,46");
        }

        configurator: FlatNetworkConfigurator
        {
            networkAddress = "145.236.0.0";
            netmask = "255.255.0.0";
            @display("p=140,50");
        }
}
