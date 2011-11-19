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
<#if parametric>
        int numOfHosts;
</#if>
        int playgroundSizeX;
        int playgroundSizeY;

    submodules:
<#if parametric>
        host[numOfHosts]: WirelessHostSimplified
        {
            @display("r=,,#707070");
        }
<#else>
    <#list 0..numOfHosts?number-1 as i>
        host${i}: WirelessHostSimplified
        {
            @display("r=,,#707070");
        }
    </#list>
</#if>

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
