<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">
package ${nedPackageName};
</#if>

// numOfHosts: ${numOfHosts}

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;


network ${targetTypeName}
{
    parameters:
<#if parametric>
        int numOfHosts;
</#if>

    submodules:
<#if parametric>
        host[numOfHosts]: WirelessHost
        {
            @display("r=,,#707070");
        }
<#else>
    <#list 0..numOfHosts?number-1 as i>
        host${i}: WirelessHost
        {
            @display("r=,,#707070");
        }
    </#list>
</#if>

        ap: AccessPoint
        {
            @display("p=213,174;r=,,#707070");
        }

        channelControl: ChannelControl
        {
            numChannels = 2;
            @display("p=61,46");
        }

        configurator: IPv4NetworkConfigurator
        {
            @display("p=140,50");
        }
}
