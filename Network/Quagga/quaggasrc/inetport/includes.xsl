<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:output method="text" indent="yes" encoding="iso-8859-1"/>

<xsl:template match="/sources">
  <xsl:message>Processing includes...</xsl:message>
  <xsl:document href="06_incl.patch" method="text">
    <xsl:apply-templates select="doxygen/compounddef[@kind='file']"/>
  </xsl:document>
</xsl:template>

<xsl:template match="compounddef">
  <xsl:for-each select="includes[@local='no']">
    <xsl:text>I;</xsl:text>
    <xsl:value-of select="../compoundname"/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="."/>
    <xsl:call-template name="newline"/>
  </xsl:for-each>
</xsl:template>

<xsl:template name="newline"><xsl:text>
</xsl:text></xsl:template>

</xsl:stylesheet>
