//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author Jan Zavrel (honza.zavrel96@gmail.com)
// @author Jan Bloudicek (jbloudicek@gmail.com)
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

// types of timers
enum EigrpTimerType
{
    EIGRP_HELLO_TIMER = 1;      // Hello timer
    EIGRP_HOLD_TIMER = 2;       // Hold timer
//    EIGRP_PACING_TIMER = 3;    // Pacing timer
};

// general timer structure
message EigrpTimer
{
    char timerKind enum(EigrpTimerType);
};
