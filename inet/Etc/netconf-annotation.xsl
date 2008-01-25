<?xml version='1.0'?>
<!-- $Header$ -->
<!-- Copyright (C) 2002 by Johnny Lai -->
<!--
 This file is part of IPv6Suite

IPv6Suite is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

IPv6Suite is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Example usage:
xsltproc -o netconf.html \-\-nonet netconf-annotation.xsl netconf.xsd
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
  xmlns:xsd="http://www.w3.org/2001/XMLSchema">
<xsl:output method="html" indent="yes"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/">
<html>
<head>
<title>Extracting content of &lt;xsd:annotations&gt; elements using XSLT.</title>
<body>
<h3>This documentation has been created using an XSLT stylesheet to process &lt;xsd:annotation&gt; elements in an XSD Schema document.</h3>
<xsl:apply-templates select="//xsd:documentation"/>
</body>
</head>
</html>
</xsl:template>

<xsl:template match="xsd:documentation">
<xsl:choose>
<xsl:when test="ancestor::*[position()=2]/@name">
  <p><b>Documentation for the &lt;<xsl:value-of select="ancestor::*[position()=2]/@name"/>&gt; element.</b><br />
<xsl:value-of select="."/></p>
</xsl:when>
<xsl:otherwise>
<p><b>Documentation for the anonymous &lt;<xsl:value-of select="name(ancestor::*[position()=2])"/>&gt; element.</b><br/>
<xsl:value-of select="."/></p>
</xsl:otherwise>
</xsl:choose>
</xsl:template>
</xsl:stylesheet>
