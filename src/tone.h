#ifndef TONE_H
#define TONE_H

#include <QFile>

#include <stdint.h>

#define TONE_BUFFER_LEN    1024     // size of circular buffer

#define TONE_RATE_ONE_HZ   65
#define TONE_RATE_FLATLINE 0xffffU

#define TONE_LENGTH_125_MS 3906
#define TONE_MAX_PITCH     65280

#define TONE_CHIRP_MAX     (((uint32_t) 3242 << 16) / TONE_LENGTH_125_MS)

class Config;

class Tone
{
public:
    Tone(const Config &config);

    uint8_t sample();

    void init();
    void update();

    void setRate(uint16_t rate);
    void setPitch(uint16_t index);
    void setChirp(uint32_t chirp);

    void task();

    void beep(uint16_t index, uint32_t chirp, uint16_t len);
    void play(QString filename);
    void wait();
    void stop();

    uint8_t canWrite();
    uint8_t isIdle();

    void hold();
    void release();

private:
    const Config &mConfig;

    uint16_t mRead;
    uint16_t mWrite;

    uint32_t mStep;
    uint32_t mChirp;
    uint16_t mLen;

    uint8_t  mState;
    uint8_t  mMode;

    QFile    mFile;

    uint16_t mNextIndex;
    uint32_t mNextChirp;
    uint16_t mRate;

    uint8_t  mFlags;
    uint8_t  mHold;

    uint32_t mWavSamples;

    uint16_t mToneTimer;
    uint16_t mPhase;
    uint8_t  mSampleCount;
    uint16_t mSampleBegin, mSampleEnd, mSampleStep;

    uint8_t  mBuffer[TONE_BUFFER_LEN];

    bool     mSampleActive;

    void loadTable(void);
    void readFile(uint16_t size);
    void loadWAV(void);
    void load(void);
    void start(uint8_t mode);

    static const uint8_t mSineTable[];
};

#endif // TONE_H
