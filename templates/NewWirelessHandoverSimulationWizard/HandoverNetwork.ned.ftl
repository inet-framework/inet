<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">
package ${nedPackageName};
</#if>

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;

network ${targetTypeName}
{
    parameters:

    submodules:
        host: WirelessHost
        {
            @display("p=50,130;r=,,#707070");
        }

<#list 1..numAccessPoints?number as i>
        ap${i}: AccessPoint
        {
            @display("p=${i*300-200},200;r=,,#707070");
        }
</#list>

        channelControl: ChannelControl
        {
            @display("p=60,50");
        }

        configurator: IPv4NetworkConfigurator
        {
            @display("p=140,50");
        }
}
