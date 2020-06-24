#ifndef UBX_H
#define UBX_H

#include <stdint.h>

class Config;
class DataPoint;
class Tone;

class UBX
{
public:
    UBX(Config &config, Tone &tone);

    void receiveMessage(const DataPoint &dp);
    void task();

private:
    typedef struct
    {
        int32_t  lon;      // Longitude                    (deg)
        int32_t  lat;      // Latitude                     (deg)
        int32_t  hMSL;     // Height above mean sea level  (mm)
        uint32_t hAcc;     // Horizontal accuracy estimate (mm)
        uint32_t vAcc;     // Vertical accuracy estimate   (mm)

        uint8_t  gpsFix;   // GPS fix type
        uint8_t  numSV;    // Number of SVs in solution

        int32_t  velN;     // North velocity               (cm/s)
        int32_t  velE;     // East velocity                (cm/s)
        int32_t  velD;     // Down velocity                (cm/s)
        int32_t  speed;    // 3D speed                     (cm/s)
        int32_t  gSpeed;   // Ground speed                 (cm/s)
        int32_t  heading;  // 2D heading                   (deg)
        uint32_t sAcc;     // Speed accuracy estimate      (cm/s)
        uint32_t cAcc;     // Heading accuracy estimate    (deg)

        int32_t  nano;     // Nanoseconds of second        (ns)
        uint16_t year;     // Year                         (1999..2099)
        uint8_t  month;    // Month                        (1..12)
        uint8_t  day;      // Day of month                 (1..31)
        uint8_t  hour;     // Hour of day                  (0..23)
        uint8_t  min;      // Minute of hour               (0..59)
        uint8_t  sec;      // Second of minute             (0..59)
    }
    UBX_saved_t ;

    Config &mConfig;
    Tone &mTone;

    uint8_t  mCurSpeech;

    uint16_t mSpCounter;

    uint8_t  mFlags;
    uint8_t  mPrevFlags;

    int32_t  mPrevHMSL;

    uint8_t  mSuppressTone;

    char     mSpeechBuf[16];
    char     *mSpeechPtr;

    int32_t  mVal[3];

    static const uint16_t mSasTable[12];

    char *writeInt32ToBuf(char *ptr, int32_t val, int8_t dec, int8_t dot, char delimiter);
    void setTone(int32_t val_1, int32_t min_1, int32_t max_1, int32_t val_2, int32_t min_2, int32_t max_2);
    void getValues(UBX_saved_t *current, uint8_t mode, int32_t *val, int32_t *min, int32_t *max);
    char *numberToSpeech(int32_t number, char *ptr);
    void speakValue(UBX_saved_t *current);
    void updateAlarms(UBX_saved_t *current);
    void updateTones(UBX_saved_t *current);
    void receiveMessage(UBX_saved_t *current);
};

#endif // UBX_H
