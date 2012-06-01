<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<!-- transform old OSPF.xml to new OSPF.xml -->

<!--
usage:
    xsltproc this_xsl_file old_xml_file >new_xml_file
-->

<!-- AddressRange -->
<xsl:template match="Area/AddressRange/Address" mode="toAttr">
  <xsl:attribute name="address"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="Area/AddressRange/Mask" mode="toAttr">
  <xsl:attribute name="mask"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="Area/AddressRange/Status" mode="toAttr">
  <xsl:attribute name="status"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="Area/AddressRange/Address|Area/AddressRange/Mask|Area/AddressRange/Status">
</xsl:template>


<!-- BroadcastInterface -->
<xsl:template match="BroadcastInterface/AreaID" mode="toAttr">
  <xsl:attribute name="areaID"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/InterfaceOutputCost" mode="toAttr">
  <xsl:attribute name="interfaceOutputCost"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/RetransmissionInterval" mode="toAttr">
  <xsl:attribute name="retransmissionInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/InterfaceTransmissionDelay" mode="toAttr">
  <xsl:attribute name="interfaceTransmissionDelay"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/RouterPriority" mode="toAttr">
  <xsl:attribute name="routerPriority"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/HelloInterval" mode="toAttr">
  <xsl:attribute name="helloInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/RouterDeadInterval" mode="toAttr">
  <xsl:attribute name="routerDeadInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/AuthenticationType" mode="toAttr">
  <xsl:attribute name="authenticationType"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="authenticationKey"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/AreaID|BroadcastInterface/InterfaceOutputCost|BroadcastInterface/RetransmissionInterval|BroadcastInterface/InterfaceTransmissionDelay|BroadcastInterface/RouterPriority|BroadcastInterface/HelloInterval|BroadcastInterface/RouterDeadInterval|BroadcastInterface/AuthenticationType|BroadcastInterface/AuthenticationKey">
</xsl:template>


<!-- PointToPointInterface -->
<xsl:template match="PointToPointInterface/AreaID" mode="toAttr">
  <xsl:attribute name="areaID"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/InterfaceOutputCost" mode="toAttr">
  <xsl:attribute name="interfaceOutputCost"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/RetransmissionInterval" mode="toAttr">
  <xsl:attribute name="retransmissionInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/InterfaceTransmissionDelay" mode="toAttr">
  <xsl:attribute name="interfaceTransmissionDelay"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/RouterPriority" mode="toAttr">
  <xsl:attribute name="routerPriority"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/HelloInterval" mode="toAttr">
  <xsl:attribute name="helloInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/RouterDeadInterval" mode="toAttr">
  <xsl:attribute name="routerDeadInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/AuthenticationType" mode="toAttr">
  <xsl:attribute name="authenticationType"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="authenticationKey"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/AreaID|PointToPointInterface/InterfaceOutputCost|PointToPointInterface/RetransmissionInterval|PointToPointInterface/InterfaceTransmissionDelay|PointToPointInterface/RouterPriority|PointToPointInterface/HelloInterval|PointToPointInterface/RouterDeadInterval|PointToPointInterface/AuthenticationType|PointToPointInterface/AuthenticationKey">
</xsl:template>


<!-- PointToMultiPointInterface -->
<xsl:template match="PointToMultiPointInterface/AreaID" mode="toAttr">
  <xsl:attribute name="areaID"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/InterfaceOutputCost" mode="toAttr">
  <xsl:attribute name="interfaceOutputCost"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/RetransmissionInterval" mode="toAttr">
  <xsl:attribute name="retransmissionInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/InterfaceTransmissionDelay" mode="toAttr">
  <xsl:attribute name="interfaceTransmissionDelay"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/RouterPriority" mode="toAttr">
  <xsl:attribute name="routerPriority"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/HelloInterval" mode="toAttr">
  <xsl:attribute name="helloInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/RouterDeadInterval" mode="toAttr">
  <xsl:attribute name="routerDeadInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/AuthenticationType" mode="toAttr">
  <xsl:attribute name="authenticationType"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="authenticationKey"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/AreaID|PointToMultiPointInterface/InterfaceOutputCost|PointToMultiPointInterface/RetransmissionInterval|PointToMultiPointInterface/InterfaceTransmissionDelay|PointToMultiPointInterface/RouterPriority|PointToMultiPointInterface/HelloInterval|PointToMultiPointInterface/RouterDeadInterval|PointToMultiPointInterface/AuthenticationType|PointToMultiPointInterface/AuthenticationKey">
</xsl:template>


<!-- NBMAInterface -->
<xsl:template match="NBMAInterface/AreaID" mode="toAttr">
  <xsl:attribute name="areaID"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/InterfaceOutputCost" mode="toAttr">
  <xsl:attribute name="interfaceOutputCost"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/RetransmissionInterval" mode="toAttr">
  <xsl:attribute name="retransmissionInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/InterfaceTransmissionDelay" mode="toAttr">
  <xsl:attribute name="interfaceTransmissionDelay"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/RouterPriority" mode="toAttr">
  <xsl:attribute name="routerPriority"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/HelloInterval" mode="toAttr">
  <xsl:attribute name="helloInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/RouterDeadInterval" mode="toAttr">
  <xsl:attribute name="routerDeadInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/AuthenticationType" mode="toAttr">
  <xsl:attribute name="authenticationType"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="authenticationKey"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/AreaID|NBMAInterface/InterfaceOutputCost|NBMAInterface/RetransmissionInterval|NBMAInterface/InterfaceTransmissionDelay|NBMAInterface/RouterPriority|NBMAInterface/HelloInterval|NBMAInterface/RouterDeadInterval|NBMAInterface/AuthenticationType|NBMAInterface/AuthenticationKey">
</xsl:template>


<!-- HostInterface -->
<xsl:template match="HostInterface/AreaID" mode="toAttr">
  <xsl:attribute name="areaID"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="HostInterface/AttachedHost" mode="toAttr">
  <xsl:attribute name="attachedHost"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="HostInterface/LinkCost" mode="toAttr">
  <xsl:attribute name="linkCost"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="HostInterface/AreaID|HostInterface/AttachedHost|HostInterface/LinkCost">
</xsl:template>


<!-- VirtualLink -->
<xsl:template match="VirtualLink/TransitAreaID" mode="toAttr">
  <xsl:attribute name="transitAreaID"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/RetransmissionInterval" mode="toAttr">
  <xsl:attribute name="retransmissionInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/InterfaceTransmissionDelay" mode="toAttr">
  <xsl:attribute name="interfaceTransmissionDelay"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/HelloInterval" mode="toAttr">
  <xsl:attribute name="helloInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/RouterDeadInterval" mode="toAttr">
  <xsl:attribute name="routerDeadInterval"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/AuthenticationType" mode="toAttr">
  <xsl:attribute name="authenticationType"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="authenticationKey"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/TransitAreaID|VirtualLink/RetransmissionInterval|VirtualLink/InterfaceTransmissionDelay|VirtualLink/HelloInterval|VirtualLink/RouterDeadInterval|VirtualLink/AuthenticationType|VirtualLink/AuthenticationKey">
</xsl:template>


<!-- ExternalInterface -->
<xsl:template match="ExternalInterface/ForwardingAddress" mode="toAttr">
  <xsl:attribute name="forwardingAddress"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="ExternalInterface/ExternalRouteTag" mode="toAttr">
  <xsl:attribute name="externalRouteTag"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="ExternalInterface/ForwardingAddress|ExternalInterface/ExternalRouteTag">
</xsl:template>

<xsl:template match="ExternalInterface/AdvertisedExternalNetwork" mode="toAttr">
  <xsl:for-each select="Address|Mask">
    <xsl:attribute name="advertisedExternalNetwork{name()}"><xsl:value-of select="." /></xsl:attribute>
  </xsl:for-each>
</xsl:template>
<xsl:template match="ExternalInterface/AdvertisedExternalNetwork">
  <xsl:for-each select="*[local-name()!='Address' and local-name()!='Mask']">
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="ExternalInterface/ExternalInterfaceOutputParameters" mode="toAttr">
  <xsl:for-each select="ExternalInterfaceOutputCost">
  <xsl:attribute name="externalInterfaceOutputCost"><xsl:value-of select="." /></xsl:attribute>
  </xsl:for-each>
  <xsl:for-each select="ExternalInterfaceOutputType">
  <xsl:attribute name="externalInterfaceOutputType"><xsl:value-of select="." /></xsl:attribute>
  </xsl:for-each>
</xsl:template>
<xsl:template match="ExternalInterface/ExternalInterfaceOutputParameters">
  <xsl:for-each select="*[local-name()!='ExternalInterfaceOutputType' and local-name()!='ExternalInterfaceOutputCost']">
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</xsl:template>


<!-- RFC1583Compatible -->
<xsl:template match="Router/RFC1583Compatible" mode="toAttr">
  <xsl:attribute name="RFC1583Compatible">true</xsl:attribute>
</xsl:template>
<xsl:template match="Router/RFC1583Compatible">
</xsl:template>

<xsl:template match="Router">
  <xsl:copy>
    <xsl:apply-templates  select="@*"/>
    <xsl:apply-templates  select="RFC1583Compatible" mode="toAttr"/>
    <xsl:apply-templates  select="node()"/>
  </xsl:copy>
</xsl:template>



<!-- do not convert other nodes to attribute -->
<xsl:template match="*" mode="toAttr">
</xsl:template>



<!-- -->
<xsl:template match="AddressRange|BroadcastInterface|PointToPointInterface|PointToMultiPointInterface|NBMAInterface|HostInterface|VirtualLink|ExternalInterface">
<!--
<xsl:template match="*">
-->
  <xsl:element name="{name()}">
    <xsl:for-each select="@*">
      <xsl:attribute name="{local-name()}"><xsl:value-of select="." /></xsl:attribute>
    </xsl:for-each>
    <xsl:for-each select="*">
      <xsl:apply-templates select="." mode="toAttr"/>
    </xsl:for-each>
    <xsl:for-each select="*">
      <xsl:apply-templates select="."/>
    </xsl:for-each>
  </xsl:element>
</xsl:template>


<xsl:template match="@*" mode="toAttr">
  <xsl:copy>
    <xsl:apply-templates select="@*"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="@*|node()">
  <xsl:copy>
    <xsl:apply-templates select="@*|node()"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>

