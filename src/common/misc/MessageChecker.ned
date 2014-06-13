//
// Copyright (C) 2010 Helene Lageber
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.common.misc;

//
// Module that allows checking fields of messages
//
// Example of XML configuration:
//
// <pre>
//  <message occurence="i">     # check i successive messages
//      <fieldValue name="<fieldname>" value="<fieldvalue>" [ type="<fieldtype>" ]/>
//      <fieldObject name="<fieldname>" [ type="<objecttype>" ]>
//          ...
//      </field>
//      <fieldArray name="<fieldname>" [ size="<arraysize>" type="<fieldtype>" ]/>
//      <fieldValueInArray name="<fieldname>" index="<arrayindex>" value="<fieldvalue>" [ size="<arraysize>" type="<objecttype>" ]/>
//      <fieldObjectInArray name="<fieldname>" [ index="<arrayindex>" size="<arraysize>" type="<objecttype>" ]>
//          ...
//      </field>
//  </message>
//  <message occurence="n"/>    # ignore n messages
// </pre>
//
// Optional attributes are surrounded with square brackets.
// See also validation schema (XSD, DTD)
//
simple MessageChecker
{
    parameters:
        xml config;   // Configuration file
    gates:
        input in;
        output out;
}
