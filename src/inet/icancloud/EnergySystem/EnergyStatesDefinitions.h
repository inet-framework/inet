/*
 * All the files import this file. The shared states between a component cannot repeat the value!!
 */

//-------------------------------------- CPU C-STATES ------------------------------------ //

#define		OFF 						"off"							// The cpu is completely off.
#define 	C0_OperatingState			"c0_operating_state"			// The cpu is fully turned on (Here actuates the P-states)
#define 	C1_Halt						"c1_halt"						// Stops CPU main internal clocks via software; bus interface unit and APIC are kept running at full speed.
#define 	C2_StopGrant				"c2_stop_grant"					// Stops CPU main internal clocks via hardware and reduces CPU voltage; bus interface unit and APIC are kept running at full speed.
#define 	C3_Sleep					"c3_sleep"						// Stops all CPU internal and external clocks
#define 	C4_DeeperSleep				"c4_deeper_sleep"				// Reduces CPU voltage
#define 	C5_EnhancedDeeperSleep		"c5_enhanced_deeper_sleep"		// Reduces CPU voltage even more and turns off the memory cache
#define		C6_DeepPowerDown			"c6_deep_power_down"		    // Reduces the CPU internal voltage to any value, including 0 V

//-------------------------------------- CPU P-STATES ------------------------------------ //

// P-States are 12. As example, the processor is: Intel(R) Core(TM) i3 CPU  M 370  @ 2.40GHz
//cpufreq stats: 2.40 GHz 2.27 GHz 2.13 GHz 2.00 GHz 1.87 GHz 1.73 GHz 1.60 GHz 1.47 GHz 1.33 GHz 1.20 GHz 1.07 GHz 933 MHz

#define		C0_P0						"c0_p0"							// 2.40 GHz. The maximum speed.
#define		C0_P1						"c0_p1"							// 2.27 GHz
#define		C0_P2						"c0_p2"							// 2.13 GHz
#define		C0_P3						"c0_p3"							// 2.00 GHz
#define		C0_P4						"c0_p4"							// 1.87 GHz
#define		C0_P5						"c0_p5"							// 1.73 GHz
#define		C0_P6						"c0_p6"							// 1.60 GHz
#define		C0_P7						"c0_p7"							// 1.47 GHz
#define		C0_P8						"c0_p8"							// 1.33 GHz
#define		C0_P9						"c0_p9"							// 1.20 GHz
#define		C0_P10						"c0_p10"						// 1.07 GHz
#define		C0_P11						"c0_p11"						// 933 GHz

//----------------------------------------- MEMORY ----------------------------------------- //

#define		MEMORY_STATE_IDLE				"memory_idle"  					// Memory OFF
#define		MEMORY_STATE_READ				"memory_read" 					// Memory OFF
#define		MEMORY_STATE_WRITE				"memory_write" 					// Memory OFF
#define		MEMORY_STATE_OFF				"memory_off"  					// Memory ON
#define		MEMORY_STATE_SEARCHING			"memory_search"  					// Memory ON

	//-------------------------------------- DDR3 ------------------------------------------//

#define		IDD0						"IDD0"							// Operating current: One bank active-precharge
#define		IDD2P						"IDD2P"							// Precharge power-down current (fast PDN Exit)
#define		IDD2N						"IDD2N"							// Precharge standby current
#define		IDD3P						"IDD3P"							// Active power-down current
#define		IDD3N						"IDD3N"							// Active standby current
#define		IDD4R						"IDD4R"							// Operating standby current
#define		IDD4W						"IDD4W"							// Operating burst write current
#define		IDD5						"IDD5"							// Burst refresh current

//----------------------------------------- DISK ----------------------------------------- //

#define		DISK_ON						"disk_on"	 					// Disk ON
#define		DISK_OFF					"disk_off"	 					// Disk OFF
#define		DISK_ACTIVE					"disk_active"    					// Disk ACTIVE

namespace inet {

namespace icancloud {

} // namespace icancloud
} // namespace inet

#define		DISK_IDLE					"disk_idle"    					// Disk IDLE

//----------------------------------------- NETWORK ----------------------------------------- //



////----------------------------------------- PSU ----------------------------------------- //

//root@:/# cpufreq-info -c 3
//cpufrequtils 007: cpufreq-info (C) Dominik Brodowski 2004-2009
//Report errors and bugs to cpufreq@vger.kernel.org, please.
//analyzing CPU 3:
//  driver: acpi-cpufreq
//  CPUs which run at the same hardware frequency: 0 1 2 3
//  CPUs which need to have their frequency coordinated by software: 3
//  maximum transition latency: 10.0 us.
//  hardware limits: 933 MHz - 2.40 GHz
//  available frequency steps: 2.40 GHz, 2.27 GHz, 2.13 GHz, 2.00 GHz, 1.87 GHz, 1.73 GHz, 1.60 GHz, 1.47 GHz, 1.33 GHz, 1.20 GHz, 1.07 GHz, 933 MHz
//  available cpufreq governors: conservative, ondemand, userspace, powersave, performance
//  current policy: frequency should be within 933 MHz and 2.40 GHz.
//                  The governor "ondemand" may decide which speed to use
//                  within this range.
//  current CPU frequency is 933 MHz (asserted by call to hardware).
//  cpufreq stats: 2.40 GHz:7,61%, 2.27 GHz:0,10%, 2.13 GHz:0,10%, 2.00 GHz:0,09%, 1.87 GHz:0,09%, 1.73 GHz:0,10%, 1.60 GHz:0,11%, 1.47 GHz:0,13%, 1.33 GHz:0,12%, 1.20 GHz:0,14%, 1.07 GHz:0,14%, 933 MHz:91,29%  (55235)



/*Available governors:

cpufreq_ondemand (default and recommended)
    Dynamically switches between the CPU(s) available clock speeds based on system load
cpufreq_performance
    The performance governor runs the CPU(s) at maximum clock speed
cpufreq_conservative
    Similar to ondemand, but the CPU(s) clock speed switches gradually through all its available frequencies based on system load
cpufreq_powersave
    Runs the CPU(s) at minimum speed
cpufreq_userspace
    Manually configured clock speeds by user
*/

