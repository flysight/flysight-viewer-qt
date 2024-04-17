#include "config.h"

#include <QFile>
#include <QMessageBox>
#include <QObject>

#define CONFIG_FIRST_ALARM  0x01
#define CONFIG_FIRST_WINDOW 0x02
#define CONFIG_FIRST_SPEECH 0x04

Config::Config():
    mAudioFolder(":/audio")
{
    reset();
}

void Config::readSingle(
        QString fileName)
{
    int first = 0;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    while (!file.atEnd())
    {
        QString line = file.readLine();

        int pos = line.indexOf(';');
        if (pos >= 0)
        {
            line = line.left(pos);
        }

        QStringList cols = line.split(":");
        if (cols.length() < 2) continue;

        QString name = cols[0].trimmed();
        QString result = cols[1].trimmed();
        int val = result.toInt();

        #define HANDLE_VALUE(s,w,r,t) \
            if ((t) && !name.compare(s)) { (w) = (r); }

        HANDLE_VALUE("Model",     UBX_model,        val, val >= 0 && val <= 8);
        HANDLE_VALUE("Rate",      UBX_rate,         val, val >= 40 && val <= 1000);
        HANDLE_VALUE("Mode",      UBX_mode,         val, (val >= 0 && val <= 7) || (val == 11));
        HANDLE_VALUE("Min",       UBX_min,          val, true);
        HANDLE_VALUE("Max",       UBX_max,          val, true);
        HANDLE_VALUE("Limits",    UBX_limits,       val, val >= 0 && val <= 2);
        HANDLE_VALUE("Volume",    Tone_volume,      8 - val, val >= 0 && val <= 8);
        HANDLE_VALUE("Mode_2",    UBX_mode_2,       val, (val >= 0 && val <= 9) || (val == 11));
        HANDLE_VALUE("Min_Val_2", UBX_min_2,        val, true);
        HANDLE_VALUE("Max_Val_2", UBX_max_2,        val, true);
        HANDLE_VALUE("Min_Rate",  UBX_min_rate,     val * TONE_RATE_ONE_HZ / 100, val >= 0);
        HANDLE_VALUE("Max_Rate",  UBX_max_rate,     val * TONE_RATE_ONE_HZ / 100, val >= 0);
        HANDLE_VALUE("Flatline",  UBX_flatline,     val, val == 0 || val == 1);
        HANDLE_VALUE("Sp_Rate",   UBX_sp_rate,      val * 1000, val >= 0 && val <= 32);
        HANDLE_VALUE("Sp_Volume", Tone_sp_volume,   8 - val, val >= 0 && val <= 8);
        HANDLE_VALUE("V_Thresh",  UBX_threshold,    val, true);
        HANDLE_VALUE("H_Thresh",  UBX_hThreshold,   val, true);
        HANDLE_VALUE("Use_SAS",   UBX_use_sas,      val, val == 0 || val == 1);
        HANDLE_VALUE("Window",    UBX_alarm_window_above, val * 1000, true);
        HANDLE_VALUE("Window",    UBX_alarm_window_below, val * 1000, true);
        HANDLE_VALUE("Win_Above", UBX_alarm_window_above, val * 1000, true);
        HANDLE_VALUE("Win_Below", UBX_alarm_window_below, val * 1000, true);
        HANDLE_VALUE("Lat",       UBX_lat,          val, val >= -900000000 && val <= 900000000);
        HANDLE_VALUE("Lon",       UBX_lon,          val, val >= -1800000000 && val <= 1800000000);
        HANDLE_VALUE("Bearing",   UBX_bearing,      val, val >= 0 && val <= 360);
        HANDLE_VALUE("End_Nav",   UBX_end_nav,      val * 1000, true);
        HANDLE_VALUE("Max_Dist",  UBX_max_dist,     val, val >= 0 && val <= 10000);
        HANDLE_VALUE("Min_Angle", UBX_min_angle,    val, val >= 0 && val <= 360);
        HANDLE_VALUE("DZ_Elev",   UBX_dz_elev,      val * 1000, true);
        HANDLE_VALUE("Init_Mode", UBX_init_mode,    val, val >= 0 && val <= 2);
        HANDLE_VALUE("Alt_Units", UBX_alt_units,    val, val >= 0 && val <= 1);
        HANDLE_VALUE("Alt_Step",  UBX_alt_step,     val, val >= 0);

        #undef HANDLE_VALUE

        if (!name.compare("Init_File"))
        {
            UBX_init_filename = result.left(8);
        }

        if (!name.compare("Alarm_Elev") && UBX_alarms.length() < UBX_MAX_ALARMS)
        {
            if (!(first & CONFIG_FIRST_ALARM))
            {
                UBX_alarms.clear();
                first |= CONFIG_FIRST_ALARM;
            }

            UBX_alarm_t alarm;
            alarm.elev = val * 1000;
            alarm.type = 0;
            UBX_alarms.append(alarm);
        }
        if (!name.compare("Alarm_Type") && UBX_alarms.length() <= UBX_MAX_ALARMS)
        {
            UBX_alarms.last().type = val;
        }
        if (!name.compare("Alarm_File") && UBX_alarms.length() <= UBX_MAX_ALARMS)
        {
            UBX_alarms.last().filename = result.left(8);
        }

        if (!name.compare("Win_Top") && UBX_windows.length() < UBX_MAX_WINDOWS)
        {
            if (!(first & CONFIG_FIRST_WINDOW))
            {
                UBX_windows.clear();
                first |= CONFIG_FIRST_WINDOW;
            }

            UBX_window_t window;
            window.top = val * 1000;
            window.bottom = val * 1000;
            UBX_windows.append(window);
        }
        if (!name.compare("Win_Bottom") && UBX_windows.length() <= UBX_MAX_WINDOWS)
        {
            UBX_windows.last().bottom = val * 1000;
        }

        if (!name.compare("Sp_Mode") && UBX_speech.length() < UBX_MAX_SPEECH)
        {
            if (!(first & CONFIG_FIRST_SPEECH))
            {
                UBX_speech.clear();
                first |= CONFIG_FIRST_SPEECH;
            }

            UBX_speech_t speech;
            speech.mode = val;
            speech.units = UBX_UNITS_MPH;
            speech.decimals = 1;
            UBX_speech.append(speech);
        }
        if (!name.compare("Sp_Units") && UBX_speech.length() <= UBX_MAX_SPEECH)
        {
            UBX_speech.last().units = val;
        }
        if (!name.compare("Sp_Dec") && UBX_speech.length() <= UBX_MAX_SPEECH)
        {
            UBX_speech.last().decimals = val;
        }
    }
}

void Config::reset()
{
    UBX_model         = 7;
    UBX_rate          = 200;
    UBX_mode          = 2;
    UBX_min           = 0;
    UBX_max           = 300;

    UBX_mode_2        = 9;
    UBX_min_2         = 300;
    UBX_max_2         = 1500;
    UBX_min_rate      = 100;
    UBX_max_rate      = 500;
    UBX_flatline      = 0;
    UBX_limits        = 1;
    UBX_use_sas       = 1;

    UBX_speech.clear();
    UBX_cur_speech    = 0;
    UBX_sp_rate       = 0;

    UBX_alt_units     = UBX_UNITS_FEET;
    UBX_alt_step      = 0;

    UBX_init_mode     = 0;
    UBX_init_filename.clear();

    UBX_threshold     = 1000;
    UBX_hThreshold    = 0;

    UBX_alarms.clear();
    UBX_alarm_window_above = 0;
    UBX_alarm_window_below = 0;

    UBX_lat           = 0;
    UBX_lon           = 0;
    UBX_bearing       = 0;
    UBX_end_nav       = 0;
    UBX_max_dist      = 10000;
    UBX_min_angle     = 5;

    UBX_windows.clear();

    UBX_dz_elev       = 0;

    Tone_volume       = 2;
    Tone_sp_volume    = 0;
}
