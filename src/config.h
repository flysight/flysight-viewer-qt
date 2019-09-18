#ifndef CONFIGURATION_H
#define CONFIGURATION_H

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

    void reset();
    void readSingle(QString fileName);

    typedef struct
    {
        int          elev;
        unsigned int type;
        QString      filename;
    }
    UBX_alarm_t;

    typedef struct
    {
        int top;
        int bottom;
    }
    UBX_window_t;

    typedef struct
    {
        unsigned int mode;
        unsigned int units;
        unsigned int decimals;
    }
    UBX_speech_t;

    unsigned int Tone_volume;
    unsigned int Tone_sp_volume;

    unsigned int UBX_model;
    unsigned int UBX_rate;
    unsigned int UBX_mode;
    int          UBX_min;
    int          UBX_max;

    unsigned int UBX_mode_2;
    int          UBX_min_2;
    int          UBX_max_2;
    int          UBX_min_rate;
    int          UBX_max_rate;
    unsigned int UBX_flatline;
    unsigned int UBX_limits;
    unsigned int UBX_use_sas;

    unsigned int UBX_threshold;
    unsigned int UBX_hThreshold;

    QList< UBX_alarm_t > UBX_alarms;
    unsigned int UBX_alarm_window_above;
    unsigned int UBX_alarm_window_below;

    QList< UBX_speech_t > UBX_speech;
    unsigned int UBX_sp_rate;

    unsigned int UBX_alt_units;
    unsigned int UBX_alt_step;

    unsigned int UBX_init_mode;
    QString      UBX_init_filename;

    QList< UBX_window_t > UBX_windows;

    int          UBX_dz_elev;
};

#endif // CONFIGURATION_H
