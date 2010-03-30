///
/// @file   OpticalFrame.msg
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2009-06-30
/// 
/// @brief  Defines 'OpticalFrame' message object.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
/// 


packet OpticalFrame
{
    int lambda;         // optical wavelength on which the frame is sent
//    We can add physical layer parameters later (e.g., optical transmitted power).
}
