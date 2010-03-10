<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">
package ${nedPackageName};
</#if>

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.wireless.WirelessAP;
import inet.nodes.wireless.WirelessHost;
import inet.world.ChannelControl;


network ${targetTypeName}
{
    parameters:
        double playgroundSizeX;
        double playgroundSizeY;

    submodules:
        host: WirelessHost
        {
            @display("p=50,130;r=,,#707070");
        }

<#list 1..numAccessPoints?number as i>
        ap${i}: WirelessAP
        {
            @display("p=${i*300-200},200;r=,,#707070");
        }
</#list>

        channelcontrol: ChannelControl
        {
            playgroundSizeX = playgroundSizeX;
            playgroundSizeY = playgroundSizeY;
            @display("p=60,50");
        }

        configurator: FlatNetworkConfigurator
        {
            @display("p=140,50");
        }
}
