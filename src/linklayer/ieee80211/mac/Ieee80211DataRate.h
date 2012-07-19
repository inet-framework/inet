#ifndef __IEEE80211_DATA_RATES_H__
#define __IEEE80211_DATA_RATES_H__

/** @brief Bit rates for 802.11b */
const double BITRATES_80211b[] =
{
    1000000,
    2000000,
    5500000,
    11000000
};
#define NUM_BITERATES_80211b 4

/** @brief Bit rates for 802.11g */
const double BITRATES_80211g[] =
{
    1000000,
    2000000,
    5500000,
    6000000,
    9000000,
    11000000,
    12000000,
    18000000,
    24000000,
    36000000,
    48000000,
    54000000
};
#define NUM_BITERATES_80211g 12

/** @brief Bit rates for 802.11a */
const double BITRATES_80211a[] =
{
    6000000,
    9000000,
    12000000,
    18000000,
    24000000,
    36000000,
    48000000,
    54000000
};
#define NUM_BITERATES_80211a 8

/** @brief Bit rates for 802.11g */
const double BITRATES_80211p[] =
{
    3000000,
    4500000,
    6000000,
    9000000,
    12000000,
    18000000,
    24000000,
    27000000
};
#define NUM_BITERATES_80211p 8

#endif
