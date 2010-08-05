//
// Module allowing to check any fields of any messages
// 
// Example of XML configuration :
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
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
//
// Optional attributes are surrounded with square brackets
// See also validation schema (XSD, DTD)

package inet.base;

simple MessageChecker{
    parameters:
        xml config;   // Configuration file
    gates:
        input in;
        output out;
}
