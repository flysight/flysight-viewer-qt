#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>

#include <QList>

#define TONE_RATE_ONE_HZ 65

#define UBX_MAX_ALARMS   10
#define UBX_MAX_WINDOWS  2
#define UBX_MAX_SPEECH   3

#define UBX_UNITS_KMH    0
#define UBX_UNITS_MPH    1

#define UBX_UNITS_METERS 0
#define UBX_UNITS_FEET   1

class Config
{
public:
    Config();

    void readSingle(QString fileName);

    typedef struct
    {
        int32_t elev;
        uint8_t type;
        QString      filename;
    }
    UBX_alarm_t;

    typedef struct
    {
        int32_t top;
        int32_t bottom;
    }
    UBX_window_t;

    typedef struct
    {
        uint8_t mode;
        uint8_t units;
        int32_t decimals;
    }
    UBX_speech_t;

    uint16_t  Tone_volume;
    uint16_t  Tone_sp_volume;

    uint8_t   UBX_model;
    uint16_t  UBX_rate;
    uint8_t   UBX_mode;
    int32_t   UBX_min;
    int32_t   UBX_max;

    uint8_t   UBX_mode_2;
    int32_t   UBX_min_2;
    int32_t   UBX_max_2;
    int32_t   UBX_min_rate;
    int32_t   UBX_max_rate;
    uint8_t   UBX_flatline;
    uint8_t   UBX_limits;
    uint8_t   UBX_use_sas;

    int32_t   UBX_threshold;
    int32_t   UBX_hThreshold;

    QList< UBX_alarm_t > UBX_alarms;
    int32_t  UBX_alarm_window_above;
    int32_t  UBX_alarm_window_below;

    QList< UBX_speech_t > UBX_speech;
    uint16_t  UBX_sp_rate;

    uint8_t   UBX_alt_units;
    int32_t   UBX_alt_step;

    uint8_t   UBX_init_mode;
    QString   UBX_init_filename;

    QList< UBX_window_t > UBX_windows;

    int32_t   UBX_dz_elev;

    QString   mAudioFolder;
};

#endif // CONFIGURATION_H
