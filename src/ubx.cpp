#include "ubx.h"

#include <math.h>

#include "config.h"
#include "datapoint.h"
#include "nav.h"
#include "tone.h"

#define ABS(a)   ((a) < 0     ? -(a) : (a))
#define MIN(a,b) (((a) < (b)) ?  (a) : (b))
#define MAX(a,b) (((a) > (b)) ?  (a) : (b))

#define UBX_INVALID_VALUE   0x7fffffffL

#define UBX_ALT_MIN         1500L // Minimum announced altitude (m)

#define UBX_HAS_FIX         0x01
#define UBX_FIRST_FIX       0x02
#define UBX_SAY_ALTITUDE    0x04
#define UBX_VERTICAL_ACC    0x08

const uint16_t UBX::mSasTable[12] =
{
    1024, 1077, 1135, 1197,
    1265, 1338, 1418, 1505,
    1600, 1704, 1818, 1944
};

UBX::UBX(
        Config &config,
        Tone &tone):
    mConfig(config),
    mTone(tone)
{
    mCurSpeech = 0;

    mSpCounter = 0;

    mFlags = 0;
    mPrevFlags = 0;

    mSuppressTone = 0;

    mSpeechBuf[0] = '\0';
    mSpeechPtr = mSpeechBuf;

    mFlags = UBX_FIRST_FIX;

    mVal[0] = UBX_INVALID_VALUE;
}

char *UBX::writeInt32ToBuf(
    char    *ptr,
    int32_t val,
    int8_t  dec,
    int8_t  dot,
    char    delimiter)
{
    int32_t value = val > 0 ? val : -val;

    *--ptr = delimiter;
    while (value >   0 || dec > 0)
    {
        ldiv_t res = ldiv(value, 10);
        *--ptr = res.rem + '0';
        value = res.quot;
        if (--dec == 0 && dot)
        {
            *--ptr = '.';
        }
    }
    if (*ptr == '.' || *ptr == delimiter)
    {
        *--ptr = '0';
    }
    if (val < 0)
    {
        *--ptr = '-';
    }

    return ptr;
}

void UBX::setTone(
        int32_t val_1,
        int32_t min_1,
        int32_t max_1,
        int32_t val_2,
        int32_t min_2,
        int32_t max_2)
{
    #define UNDER(val,min,max) ((min < max) ? (val <= min) : (val >= min))
    #define OVER(val,min,max)  ((min < max) ? (val >= max) : (val <= max))

    if (val_1 != UBX_INVALID_VALUE &&
        val_2 != UBX_INVALID_VALUE)
    {
        if (UNDER(val_2, min_2, max_2))
        {
            if (mConfig.UBX_flatline)
            {
                mTone.setRate(TONE_RATE_FLATLINE);
            }
            else
            {
                mTone.setRate(mConfig.UBX_min_rate);
            }
        }
        else if (OVER(val_2, min_2, max_2))
        {
            mTone.setRate(mConfig.UBX_max_rate - 1);
        }
        else
        {
            mTone.setRate(mConfig.UBX_min_rate + (mConfig.UBX_max_rate - mConfig.UBX_min_rate) * (val_2 - min_2) / (max_2 - min_2));
        }

        if (UNDER(val_1, min_1, max_1))
        {
            if (mConfig.UBX_limits == 0)
            {
                mTone.setRate(0);
            }
            else if (mConfig.UBX_limits == 1)
            {
                mTone.setPitch(0);
                mTone.setChirp(0);
            }
            else if (mConfig.UBX_limits == 2)
            {
                mTone.setPitch(0);
                mTone.setChirp(TONE_CHIRP_MAX);
            }
            else
            {
                mTone.setPitch(TONE_MAX_PITCH - 1);
                mTone.setChirp(-TONE_CHIRP_MAX);
            }
        }
        else if (OVER(val_1, min_1, max_1))
        {
            if (mConfig.UBX_limits == 0)
            {
                mTone.setRate(0);
            }
            else if (mConfig.UBX_limits == 1)
            {
                mTone.setPitch(TONE_MAX_PITCH - 1);
                mTone.setChirp(0);
            }
            else if (mConfig.UBX_limits == 2)
            {
                mTone.setPitch(TONE_MAX_PITCH - 1);
                mTone.setChirp(-TONE_CHIRP_MAX);
            }
            else
            {
                mTone.setPitch(0);
                mTone.setChirp(TONE_CHIRP_MAX);
            }
        }
        else
        {
            mTone.setPitch(TONE_MAX_PITCH * (val_1 - min_1) / (max_1 - min_1));
            mTone.setChirp(0);
        }
    }
    else
    {
        mTone.setRate(0);
    }

    #undef OVER
    #undef UNDER
}

void UBX::getValues(
        UBX_saved_t *current,
        uint8_t mode,
        int32_t *val,
        int32_t *min,
        int32_t *max)
{
    uint16_t speed_mul = 1024;

    if (mConfig.UBX_use_sas)
    {
        if (current->hMSL < 0)
        {
            speed_mul = mSasTable[0];
        }
        else if (current->hMSL >= 11534336L)
        {
            speed_mul = mSasTable[11];
        }
        else
        {
            int32_t h = current->hMSL / 1024	;
            uint16_t i = h / 1024;
            uint16_t j = h % 1024;
            uint16_t y1 = mSasTable[i];
            uint16_t y2 = mSasTable[i + 1];
            speed_mul = y1 + ((y2 - y1) * j) / 1024;
        }
    }

    int32_t tVal;
    switch (mode)
    {
    case 0: // Horizontal speed
        *val = (current->gSpeed * 1024) / speed_mul;
        break;
    case 1: // Vertical speed
        *val = (current->velD * 1024) / speed_mul;
        break;
    case 2: // Glide ratio
        if (current->velD != 0)
        {
            *val = 10000 * (int32_t) current->gSpeed / current->velD;
            *min *= 100;
            *max *= 100;
        }
        break;
    case 3: // Inverse glide ratio
        if (current->gSpeed != 0)
        {
            *val = 10000 * current->velD / (int32_t) current->gSpeed;
            *min *= 100;
            *max *= 100;
        }
        break;
    case 4: // Total speed
        *val = (current->speed * 1024) / speed_mul;
        break;
    case 5: // Direction to destination
        //check if too far from destination for Nav, would indicate user error with Lat & Lon
        if ((Nav::calcDistance(current->lat, current->lon, mConfig.UBX_lat, mConfig.UBX_lon) < mConfig.UBX_max_dist) || (mConfig.UBX_max_dist == 0))
        {
            //check if above height tone should be silenced
            if ((current->hMSL > (mConfig.UBX_end_nav + mConfig.UBX_dz_elev)) || (mConfig.UBX_end_nav == 0))
            {
                tVal = Nav::calcDirection(mConfig, current->lat, current->lon,current->heading);
                //check if heading not within UBX_min_angle deg of bearing or tones needed for other measurement
                if ((ABS(tVal) > mConfig.UBX_min_angle) || (mConfig.UBX_mode_2 != 5) || (mConfig.UBX_min_angle == 0))
                {
                    *min = -180;
                    *max = 180;
                    //manipulate tone so biggest change is at desired heading
                    if (tVal < 0)
                    {
                        *val = -180 - tVal;
                    }
                    else
                    {
                        *val = 180 - tVal;
                    }
                }
            }
        }
        break;
    case 6: // Distance to destination
        *min = 0;
        if (mConfig.UBX_max_dist != 0 )
        {
            *max = mConfig.UBX_max_dist;
        }
        else
        {
            *max = 10000; //set a default maximum value
        }
        *val = Nav::calcDistance(current->lat, current->lon, mConfig.UBX_lat, mConfig.UBX_lon);
        if (*val < *max)
        {
            *val = *max - *val;  //make inverse so higher pitch indicates shorter distance
        }
        else
        {
            *val = 0;  //set to lowest pitch/Hz
        }
        break;
    case 7: // Direction to bearing
        //check if above height tone should be silenced
        if ((current->hMSL > (mConfig.UBX_end_nav + mConfig.UBX_dz_elev)) || (mConfig.UBX_end_nav == 0))
        {
            tVal = Nav::calcRelBearing(mConfig.UBX_bearing, current->heading);
            //check if heading not within UBX_min_angle deg of bearing or tones needed for other measurement
            if ((ABS(tVal) > mConfig.UBX_min_angle) || (mConfig.UBX_mode_2 != 7) || (mConfig.UBX_min_angle == 0))
            {
                *min = -180;
                *max = 180;
                //manipulate tone so biggest change is at desired heading
                if (tVal < 0)
                {
                    *val = -180 - tVal;
                }
                else
                {
                    *val = 180 - tVal;
                }
            }
        }
        break;
    case 10: // Left/right
        //check if too far from destination for Nav, would indicate user error with Lat & Lon
        if ((Nav::calcDistance(current->lat, current->lon, mConfig.UBX_lat, mConfig.UBX_lon) < mConfig.UBX_max_dist) || (mConfig.UBX_max_dist == 0))
        {
            //check if above height tone should be silenced
            if ((current->hMSL > (mConfig.UBX_end_nav + mConfig.UBX_dz_elev)) || (mConfig.UBX_end_nav == 0))
            {
                tVal = Nav::calcDirection(mConfig, current->lat, current->lon, current->heading);
                *min = 0;
                *max = 10;
                if (ABS(tVal) > mConfig.UBX_min_angle)
                {
                    if (tVal < 0)   //left turn required  - low pitch tone
                    {
                        *val = *min;
                    }
                    else            //right turn required - high pitch tone
                    {
                        *val = *max;
                    }
                }
                else                //mid tone
                {
                    *val = (*max - *min) / 2;
                }
            }
        }
        break;
    case 11: // Dive angle
        *val = atan2(current->velD, current->gSpeed) / M_PI * 180;
        break;
    }
}

char *UBX::numberToSpeech(
        int32_t number,
        char *ptr)
{
    // Adapted from https://stackoverflow.com/questions/2729752/converting-numbers-in-to-words-c-sharp

    if (number == 0)
    {
        *(ptr++) = '0';
        return ptr;
    }

    if (number < 0)
    {
        *(ptr++) = '-';
        return numberToSpeech(-number, ptr);
    }

    if ((number / 1000) > 0)
    {
        ptr = numberToSpeech(number / 1000, ptr);
        *(ptr++) = 'k';
        number %= 1000;
    }

    if ((number / 100) > 0)
    {
        ptr = numberToSpeech(number / 100, ptr);
        *(ptr++) = 'h';
        number %= 100;
    }

    if (number > 0)
    {
        if (number < 10)
        {
            *(ptr++) = '0' + number;
        }
        else if (number < 20)
        {
            *(ptr++) = 't';
            *(ptr++) = '0' + (number - 10);
        }
        else
        {
            *(ptr++) = 'x';
            *(ptr++) = '0' + (number / 10);

            if ((number % 10) > 0)
                *(ptr++) = '0' + (number % 10);
        }
    }

    return ptr;
}

void UBX::speakValue(UBX_saved_t *current)
{
    uint16_t speed_mul = 1024;
    int32_t step_size, step;

    char *end_ptr;

    if (mConfig.UBX_use_sas)
    {
        if (current->hMSL < 0)
        {
            speed_mul = mSasTable[0];
        }
        else if (current->hMSL >= 11534336L)
        {
            speed_mul = mSasTable[11];
        }
        else
        {
            int32_t h = current->hMSL / 1024	;
            uint16_t i = h / 1024;
            uint16_t j = h % 1024;
            uint16_t y1 = mSasTable[i];
            uint16_t y2 = mSasTable[i + 1];
            speed_mul = y1 + ((y2 - y1) * j) / 1024;
        }
    }

    switch (mConfig.UBX_speech[mCurSpeech].units)
    {
    case UBX_UNITS_KMH:
        speed_mul = (uint16_t) (((uint32_t) speed_mul * 18204) / 65536);
        break;
    case UBX_UNITS_MPH:
        speed_mul = (uint16_t) (((uint32_t) speed_mul * 29297) / 65536);
        break;
    case UBX_UNITS_KN:
        speed_mul = (uint16_t) (((uint32_t) speed_mul * 33713) / 65536);
        break;
    }

    // Step 0: Initialize speech pointers, leaving room at the end for one unit character

    mSpeechPtr = mSpeechBuf + sizeof(mSpeechBuf) - 1;
    end_ptr = mSpeechPtr;

    // Step 1: Get speech value with 2 decimal places

    int32_t tVal;
    int32_t decimals = mConfig.UBX_speech[mCurSpeech].decimals;

    switch (mConfig.UBX_speech[mCurSpeech].mode)
    {
    case 0: // Horizontal speed
        mSpeechPtr = writeInt32ToBuf(mSpeechPtr, (current->gSpeed * 1024) / speed_mul, 2, 1, 0);
        break;
    case 1: // Vertical speed
        mSpeechPtr = writeInt32ToBuf(mSpeechPtr, (current->velD * 1024) / speed_mul, 2, 1, 0);
        break;
    case 2: // Glide ratio
        if (current->velD != 0)
        {
            mSpeechPtr = writeInt32ToBuf(mSpeechPtr, 100 * (int32_t) current->gSpeed / current->velD, 2, 1, 0);
        }
        else
        {
            *(--mSpeechPtr) = 0;
        }
        break;
    case 3: // Inverse glide ratio
        if (current->gSpeed != 0)
        {
            mSpeechPtr = writeInt32ToBuf(mSpeechPtr, 100 * (int32_t) current->velD / current->gSpeed, 2, 1, 0);
        }
        else
        {
            *(--mSpeechPtr) = 0;
        }
        break;
    case 4: // Total speed
        mSpeechPtr = writeInt32ToBuf(mSpeechPtr, (current->speed * 1024) / speed_mul, 2, 1, 0);
        break;
    case 5: // Direction to destination
        //check if too far from destination for Nav, would indicate user error with Lat & Lon
        if ((Nav::calcDistance(current->lat, current->lon, mConfig.UBX_lat, mConfig.UBX_lon) < mConfig.UBX_max_dist) || (mConfig.UBX_max_dist == 0))
        {
            //check if above height tone should be silenced
            if ((current->hMSL > (mConfig.UBX_end_nav + mConfig.UBX_dz_elev)) || (mConfig.UBX_end_nav == 0))
            {
                decimals = 0;
                tVal = Nav::calcDirection(mConfig, current->lat, current->lon, current->heading);
                mSpeechPtr = writeInt32ToBuf(mSpeechPtr, ABS(tVal) * 100, 2, 1, 0);
            }
        }
        break;
    case 6: // Distance to destination
        decimals = 1;
        tVal = Nav::calcDistance(current->lat, current->lon, mConfig.UBX_lat, mConfig.UBX_lon);  // returns metres
        switch (mConfig.UBX_speech[mCurSpeech].units)
        {
        case UBX_UNITS_KMH:
            tVal = tVal / 10;
            break;
        case UBX_UNITS_MPH:
            tVal = (tVal * 100) / 1609;
            break;
        case UBX_UNITS_KN:
            tVal = (tVal * 100) / 1852;
            break;
        }
        tVal = tVal + 5; //for correct rounding when reducing to one decimal place
        mSpeechPtr = writeInt32ToBuf(mSpeechPtr, tVal, 2, 1, 0);
        break;
    case 7: // Direction to bearing
        //check if above height tone should be silenced
        if ((current->hMSL > (mConfig.UBX_end_nav + mConfig.UBX_dz_elev)) || (mConfig.UBX_end_nav == 0))
        {
            decimals = 0;
            tVal = Nav::calcRelBearing(mConfig.UBX_bearing, current->heading);
            mSpeechPtr = writeInt32ToBuf(mSpeechPtr, ABS(tVal) * 100, 2, 1, 0);
        }
        break;
    case 11: // Dive angle
        mSpeechPtr = writeInt32ToBuf(mSpeechPtr, 100 * atan2(current->velD, current->gSpeed) / M_PI * 180, 2, 1, 0);
        break;
    case 12: // Altitude
        if (mConfig.UBX_speech[mCurSpeech].units == UBX_UNITS_KMH)
        {
            step_size = 10000 * decimals;
        }
        else
        {
            step_size = 3048 * decimals;
        }
        step = ((current->hMSL - mConfig.UBX_dz_elev) * 10 + step_size / 2) / step_size;
        mSpeechPtr = mSpeechBuf + 2;
        mSpeechPtr = numberToSpeech(step * decimals, mSpeechPtr);
        end_ptr = mSpeechPtr;
        mSpeechPtr = mSpeechBuf + 2;
        break;
    }

    // Step 1.5: Include label
    if (mConfig.UBX_speech.length() > 1)
    {
        *(--mSpeechPtr) = mConfig.UBX_speech[mCurSpeech].mode + 1;
        *(--mSpeechPtr) = '>';
    }

    // Step 2: Truncate to the desired number of decimal places

    if (decimals == 0) end_ptr -= 4;
    else end_ptr -= 3 - decimals;

    // Step 3: Add units if needed, e.g., *(end_ptr++) = 'k';

    switch (mConfig.UBX_speech[mCurSpeech].mode)
    {
    case 0: // Horizontal speed
    case 1: // Vertical speed
    case 2: // Glide ratio
    case 3: // Inverse glide ratio
    case 4: // Total speed
    case 11: // Dive angle
        break;
    case 5: // Direction to destination
    case 7: // Direction to bearing
        if (tVal < 0)			*(end_ptr++) = 'l';
        else if (tVal > 0)		*(end_ptr++) = 'r';
        break;
    case 6: // Distance to destination
        switch (mConfig.UBX_speech[mCurSpeech].units)
        {
        case UBX_UNITS_MPH:
            *(end_ptr++) = 'i';
            break;
        case UBX_UNITS_KMH:
            *(end_ptr++) = 'K';
            break;
        case UBX_UNITS_KN:
            *(end_ptr++) = 'n';
            break;
        }
        break;
    case 12: // Altitude
        *(end_ptr++) = (mConfig.UBX_speech[mCurSpeech].units == UBX_UNITS_KMH) ? 'm' : 'f';
        break;
    }

    // Step 4: Terminate with a null

    *(end_ptr++) = 0;
}

void UBX::loadConfig(QString filename)
{
    mConfig.UBX_init_filename.clear();

    QString tempFilename;
    if (!mConfig.mConfigFolder.isEmpty() && !filename.isEmpty())
    {
        tempFilename = mConfig.mConfigFolder + QString("/") + filename + QString(".txt");
    }

    mConfig.reset();
    mConfig.readSingle(mConfig.mRootConfig);

    if (!tempFilename.isEmpty())
    {
        mConfig.readSingle(tempFilename);
    }

    if (!mConfig.UBX_init_filename.isEmpty())
    {
        mTone.play(mConfig.UBX_init_filename + QString(".wav"));
    }
}

void UBX::updateAlarms(UBX_saved_t *current)
{
    uint8_t i, suppress_tone, suppress_alt;
    int32_t step_size, step, step_elev;

    suppress_tone = 0;
    suppress_alt = 0;

    for (i = 0; i < mConfig.UBX_alarms.length(); ++i)
    {
        const int32_t alarm_elev = mConfig.UBX_alarms[i].elev + mConfig.UBX_dz_elev;

        if ((current->hMSL <= alarm_elev + mConfig.UBX_alarm_window_above) &&
            (current->hMSL >= alarm_elev - mConfig.UBX_alarm_window_below))
        {
            suppress_tone = 1;
            break;
        }
    }

    for (i = 0; i < mConfig.UBX_windows.length(); ++i)
    {
        if ((mConfig.UBX_windows[i].bottom + mConfig.UBX_dz_elev <= current->hMSL) &&
            (mConfig.UBX_windows[i].top + mConfig.UBX_dz_elev >= current->hMSL))
        {
            suppress_tone = 1;
            suppress_alt = 1;
            break;
        }
    }

    if (mConfig.UBX_alt_step > 0)
    {
        if (mConfig.UBX_alt_units == UBX_UNITS_METERS)
        {
            step_size = 10000 * mConfig.UBX_alt_step;
        }
        else
        {
            step_size = 3048 * mConfig.UBX_alt_step;
        }

        step = ((current->hMSL - mConfig.UBX_dz_elev) * 10 + step_size / 2) / step_size;
        step_elev = step * step_size / 10 + mConfig.UBX_dz_elev;

        if ((current->hMSL <= step_elev + mConfig.UBX_alarm_window_above) &&
            (current->hMSL >= step_elev - mConfig.UBX_alarm_window_below) &&
            (current->hMSL - mConfig.UBX_dz_elev >= UBX_ALT_MIN * 1000))
        {
            suppress_tone = 1;
        }
    }

    if (suppress_tone && !mSuppressTone)
    {
        *mSpeechPtr = 0;
        mTone.setRate(0);
        mTone.stop();
    }

    mSuppressTone = suppress_tone;

    if (mPrevFlags & UBX_HAS_FIX)
    {
        int32_t min = MIN(mPrevHMSL, current->hMSL);
        int32_t max = MAX(mPrevHMSL, current->hMSL);

        for (i = 0; i < mConfig.UBX_alarms.length(); ++i)
        {
            const int32_t alarm_elev = mConfig.UBX_alarms[i].elev + mConfig.UBX_dz_elev;

            if (alarm_elev >= min && alarm_elev < max)
            {
                switch (mConfig.UBX_alarms[i].type)
                {
                case 1:	// beep
                    mTone.beep(TONE_MAX_PITCH - 1, 0, TONE_LENGTH_125_MS);
                    break ;
                case 2:	// chirp up
                    mTone.beep(0, TONE_CHIRP_MAX, TONE_LENGTH_125_MS);
                    break ;
                case 3:	// chirp down
                    mTone.beep(TONE_MAX_PITCH - 1, -TONE_CHIRP_MAX, TONE_LENGTH_125_MS);
                    break ;
                case 4:	// play file
                    mTone.play(mConfig.UBX_alarms[i].filename + QString(".wav"));
                    break;
                case 8: // load config (ascending)
                    if (current->hMSL >= mPrevHMSL)
                    {
                        loadConfig(mConfig.UBX_alarms[i].filename);
                    }
                    break;
                case 9: // load config (descending)
                    if (current->hMSL < mPrevHMSL)
                    {
                        loadConfig(mConfig.UBX_alarms[i].filename);
                    }
                    break;
                }

                *mSpeechPtr = 0;
                break;
            }
        }

        if ((mConfig.UBX_alt_step > 0) &&
            (i == mConfig.UBX_alarms.length()) &&
            (mPrevHMSL - mConfig.UBX_dz_elev >= UBX_ALT_MIN * 1000) &&
            (*mSpeechPtr == 0) &&
            !(mFlags & UBX_SAY_ALTITUDE) &&
            !suppress_alt)
        {
            if ((step_elev >= min && step_elev < max) &&
                ABS(current->velD) >= mConfig.UBX_threshold &&
                current->gSpeed >= mConfig.UBX_hThreshold)
            {
                mSpeechPtr = mSpeechBuf;
                mSpeechPtr = numberToSpeech(step * mConfig.UBX_alt_step, mSpeechPtr);
                *(mSpeechPtr++) = (mConfig.UBX_alt_units == UBX_UNITS_METERS) ? 'm' : 'f';
                *(mSpeechPtr++) = 0;
                mSpeechPtr = mSpeechBuf;
            }
        }
    }
}

void UBX::updateTones(UBX_saved_t *current)
{
    int32_t val_1 = UBX_INVALID_VALUE, min_1 = mConfig.UBX_min, max_1 = mConfig.UBX_max;
    int32_t val_2 = UBX_INVALID_VALUE, min_2 = mConfig.UBX_min_2, max_2 = mConfig.UBX_max_2;

    uint8_t i;

    getValues(current, mConfig.UBX_mode, &val_1, &min_1, &max_1);

    switch (mConfig.UBX_mode_2)
    {
    case 5: // Direction to destination
        if (mConfig.UBX_mode == 5)  //no need to re-calculate direction
        {
            val_2 = ABS(val_1);
        }
        else
        {
            val_2 = ABS(Nav::calcDirection(mConfig, current->lat, current->lon, current->heading));
            val_2 = 180 - val_2;  //make inverse so faster rate indicates closer to bearing
        }
        val_2 = pow(val_2, 3);
        min_2 = 0;
        max_2 = pow(180, 3);
        break;
    case 7: // Direction to bearing
        if (mConfig.UBX_mode == 7)  //no need to re-calculate direction
        {
            val_2 = ABS(val_1);
        }
        else
        {
            val_2 = ABS(Nav::calcRelBearing(mConfig.UBX_bearing, current->heading));
            val_2 = 180 - val_2;  //make inverse so faster rate indicates closer to bearing
        }
        min_2 = 0;
        max_2 = 180;
        break;
    case 8: // Magnitude of value 1
        getValues(current, mConfig.UBX_mode, &val_2, &min_2, &max_2);
        if (val_2 != UBX_INVALID_VALUE)
        {
            val_2 = ABS(val_2);
        }
        break;
    case 9: // Change in value 1
        mVal[2] = mVal[1];
        mVal[1] = mVal[0];
        mVal[0] = val_1;

        if (mVal[0] != UBX_INVALID_VALUE &&
            mVal[1] != UBX_INVALID_VALUE &&
            mVal[2] != UBX_INVALID_VALUE)
        {
            val_2 = (int32_t) 1000 * (mVal[2] - mVal[0]) / (int32_t) (2 * mConfig.UBX_rate);
            val_2 = (int32_t) 10000 * ABS(val_2) / ABS(max_1 - min_1);
        }
        break;
    default:
        getValues(current, mConfig.UBX_mode_2, &val_2, &min_2, &max_2);
    }

    if (!mSuppressTone)
    {
        if (ABS(current->velD) >= mConfig.UBX_threshold &&
            current->gSpeed >= mConfig.UBX_hThreshold)
        {
            setTone(val_1, min_1, max_1, val_2, min_2, max_2);

            if (mConfig.UBX_sp_rate != 0 &&
                mConfig.UBX_speech.length() != 0 &&
                mSpCounter >= mConfig.UBX_sp_rate &&
                (*mSpeechPtr == 0) &&
                !(mFlags & UBX_SAY_ALTITUDE))
            {
                for (i = 0; i < mConfig.UBX_speech.length(); ++i)
                {
                    if ((mConfig.UBX_speech[mCurSpeech].mode != 5) ||
                        (current->hMSL - mConfig.UBX_dz_elev >= UBX_ALT_MIN * 1000))
                    {
                        speakValue(current);
                        mCurSpeech = (mCurSpeech + 1) % mConfig.UBX_speech.length();
                        break;
                    }
                    else
                    {
                        mCurSpeech = (mCurSpeech + 1) % mConfig.UBX_speech.length();
                    }
                }

                mSpCounter = 0;
            }
        }
        else
        {
            mTone.setRate(0);
        }
    }

    if (mSpCounter < mConfig.UBX_sp_rate)
    {
        mSpCounter += mConfig.UBX_rate;
    }
}

void UBX::receiveMessage(UBX_saved_t *current)
{
    mFlags |= UBX_HAS_FIX;

    updateAlarms(current);
    updateTones(current);

    if (current->vAcc < 10000)
    {
        mFlags |= UBX_VERTICAL_ACC;
    }
    else
    {
        mFlags &= ~UBX_VERTICAL_ACC;
    }

    mPrevFlags = mFlags;
    mPrevHMSL = current->hMSL;
}

void UBX::receiveMessage(
        const DataPoint &dp)
{
    UBX_saved_t current;

    current.lat = dp.lat * 1e7;
    current.lon = dp.lon * 1e7;
    current.hMSL = dp.hMSL * 1e3;
    current.hAcc = dp.hAcc * 1e3;
    current.vAcc = dp.vAcc * 1e3;

    current.gpsFix = 3;
    current.numSV = dp.numSV;

    current.velN = dp.velN * 1e2;
    current.velE = dp.velE * 1e2;
    current.velD = dp.velD * 1e2;
    current.speed = sqrt(dp.velN * dp.velN + dp.velE * dp.velE + dp.velD * dp.velD) * 1e2;
    current.gSpeed = sqrt(dp.velN * dp.velN + dp.velE * dp.velE) * 1e2;

    // Get adjusted heading
    double heading = dp.heading;
    while (heading <  0)   heading += 360;
    while (heading >= 360) heading -= 360;

    current.heading = heading * 1e5;
    current.sAcc = dp.sAcc * 1e2;
    current.cAcc = dp.cAcc * 1e5;

    current.nano = dp.dateTime.toUTC().time().msec() * 1e6;
    current.year = dp.dateTime.toUTC().date().year();
    current.month = dp.dateTime.toUTC().date().month();
    current.day = dp.dateTime.toUTC().date().day();
    current.hour = dp.dateTime.toUTC().time().hour();
    current.min = dp.dateTime.toUTC().time().minute();
    current.sec = dp.dateTime.toUTC().time().second();

    receiveMessage(&current);
}

void UBX::task()
{
    if (*mSpeechPtr)
    {
        if (mTone.isIdle())
        {
            mTone.hold();

            if (*mSpeechPtr == '-')
            {
                mTone.play("minus.wav");
            }
            else if (*mSpeechPtr == '.')
            {
                mTone.play("dot.wav");
            }
            else if (*mSpeechPtr == 'h')
            {
                mTone.play("00.wav");
            }
            else if (*mSpeechPtr == 'k')
            {
                mTone.play("000.wav");
            }
            else if (*mSpeechPtr == 'm')
            {
                mTone.play("meters.wav");
            }
            else if (*mSpeechPtr == 'f')
            {
                mTone.play("feet.wav");
            }
            else if (*mSpeechPtr == 't')
            {
                ++mSpeechPtr;
                mTone.play(QString("1") + QString(*mSpeechPtr) + QString(".wav"));
            }
            else if (*mSpeechPtr == 'x')
            {
                ++mSpeechPtr;
                mTone.play(QString(*mSpeechPtr) + QString("0") + QString(".wav"));
            }
            else if (*mSpeechPtr == '>')
            {
                ++mSpeechPtr;
                switch ((*mSpeechPtr) - 1)
                {
                    case 0:
                        mTone.play("horz.wav");
                        break;
                    case 1:
                        mTone.play("vert.wav");
                        break;
                    case 2:
                        mTone.play("glide.wav");
                        break;
                    case 3:
                        mTone.play("iglide.wav");
                        break;
                    case 4:
                        mTone.play("speed.wav");
                        break;
                    case 11:
                        mTone.play("dive.wav");
                        break;
                    case 12:
                        mTone.play("alt.wav");
                        break;
                }
            }
            else if (*mSpeechPtr == 'l')
            {
                mTone.play("left.wav");
            }
            else if (*mSpeechPtr == 'r')
            {
                mTone.play("right.wav");
            }
            else if (*mSpeechPtr == 'i')
            {
                mTone.play("miles.wav");
            }
            else if (*mSpeechPtr == 'K')
            {
                mTone.play("km.wav");
            }
            else if (*mSpeechPtr == 'n')
            {
                mTone.play("knots.wav");
            }
            else if (*mSpeechPtr == 'o')
            {
                mTone.play("oclock.wav");
            }
            else if (*mSpeechPtr == 'a')
            {
                mTone.play("10.wav");
            }
            else if (*mSpeechPtr == 'b')
            {
                mTone.play("11.wav");
            }
            else if (*mSpeechPtr == 'c')
            {
                mTone.play("12.wav");
            }
            else
            {
                mTone.play(QString(*mSpeechPtr) + QString(".wav"));
            }

            ++mSpeechPtr;
        }
    }
    else
    {
        mTone.release();

        if ((mFlags & UBX_FIRST_FIX) && mTone.isIdle())
        {
            mFlags &= ~UBX_FIRST_FIX;
            mTone.beep(TONE_MAX_PITCH - 1, 0, TONE_LENGTH_125_MS);
        }

        if ((mFlags & UBX_SAY_ALTITUDE) &&
            (mFlags & UBX_HAS_FIX) &&
            (mFlags & UBX_VERTICAL_ACC) &&
            mTone.isIdle())
        {
            mFlags &= ~UBX_SAY_ALTITUDE;
            mSpeechPtr = mSpeechBuf;

            if (mConfig.UBX_alt_units == UBX_UNITS_METERS)
            {
                mSpeechPtr = numberToSpeech((mPrevHMSL - mConfig.UBX_dz_elev) / 1000, mSpeechPtr);
                *(mSpeechPtr++) = 'm';
            }
            else
            {
                mSpeechPtr = numberToSpeech((mPrevHMSL - mConfig.UBX_dz_elev) * 10 / 3048, mSpeechPtr);
                *(mSpeechPtr++) = 'f';
            }

            *(mSpeechPtr++) = 0;
            mSpeechPtr = mSpeechBuf;
        }
    }
}
