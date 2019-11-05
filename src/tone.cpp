#include "tone.h"

#include "config.h"

#define MIN(a,b) (((a) < (b)) ?  (a) : (b))
#define MAX(a,b) (((a) > (b)) ?  (a) : (b))

#define TONE_BUFFER_CHUNK (TONE_BUFFER_LEN / 8)  // maximum bytes read in one operation
#define TONE_BUFFER_WRITE (TONE_BUFFER_LEN - TONE_BUFFER_CHUNK)  // buffered samples required to allow write/flush

#define TONE_SAMPLE_LEN  4  // number of repeated PWM samples

#define TONE_STATE_IDLE  0
#define TONE_STATE_PLAY  1

#define TONE_FLAGS_LOAD  1
#define TONE_FLAGS_STOP  2
#define TONE_FLAGS_BEEP  4

#define TONE_MODE_BEEP   0
#define TONE_MODE_WAV    1

const uint8_t Tone::mSineTable[] =
{
    128, 131, 134, 137, 140, 143, 146, 149,
    153, 156, 159, 162, 165, 168, 171, 174,
    177, 180, 182, 185, 188, 191, 194, 196,
    199, 201, 204, 207, 209, 211, 214, 216,
    218, 220, 223, 225, 227, 229, 231, 232,
    234, 236, 238, 239, 241, 242, 243, 245,
    246, 247, 248, 249, 250, 251, 252, 253,
    253, 254, 254, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 254, 254, 253,
    253, 252, 251, 251, 250, 249, 248, 247,
    245, 244, 243, 241, 240, 238, 237, 235,
    233, 232, 230, 228, 226, 224, 222, 219,
    217, 215, 213, 210, 208, 205, 203, 200,
    198, 195, 192, 189, 187, 184, 181, 178,
    175, 172, 169, 166, 163, 160, 157, 154,
    151, 148, 145, 142, 139, 135, 132, 129,
    126, 123, 120, 116, 113, 110, 107, 104,
     101, 98,  95,  92,  89,  86,  83,  80,
     77,  74,  71,  68,  66,  63,  60,  57,
     55,  52,  50,  47,  45,  42,  40,  38,
     36,  33,  31,  29,  27,  25,  23,  22,
     20,  18,  17,  15,  14,  12,  11,  10,
      8,   7,   6,   5,   4,   4,   3,   2,
      2,   1,   1,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   1,   1,   2,
      2,   3,   4,   5,   6,   7,   8,   9,
     10,  12,  13,  14,  16,  17,  19,  21,
     23,  24,  26,  28,  30,  32,  35,  37,
     39,  41,  44,  46,  48,  51,  54,  56,
     59,  61,  64,  67,  70,  73,  75,  78,
     81,  84,  87,  90,  93,  96,  99, 102,
    106, 109, 112, 115, 118, 121, 124, 128
};

Tone::Tone(const Config &config):
    mConfig(config)
{
    mState = TONE_STATE_IDLE;

    mNextIndex = 0;
    mNextChirp = 0;
    mRate = 0;

    mFlags = 0;
    mHold  = 0;

    mToneTimer = 0;
    mPhase = 0;
    mSampleCount = 0;

    mSampleActive = false;
}

uint8_t Tone::sample()
{
    if (!mSampleActive) return 128;

    if (mSampleCount++ % TONE_SAMPLE_LEN)
    {
        mSampleBegin += mSampleStep;
    }
    else if (mRead == mWrite)
    {
        if (mFlags & TONE_FLAGS_LOAD)
        {
            // Buffer underflow
            mSampleBegin = mSampleEnd;
            mSampleStep = 0;
        }
        else
        {
            // We are done playing
            mSampleActive = false;

            mFlags |= TONE_FLAGS_STOP;
        }
    }
    else
    {
        mSampleBegin = mSampleEnd;
        mSampleEnd = (uint16_t) mBuffer[mRead % TONE_BUFFER_LEN] << 8;

        // The contortions below are necessary to ensure that the division by
        // TONE_SAMPLE_LEN uses shift operations instead of calling a signed
        // integer division function.

        if (mSampleBegin <= mSampleEnd)
        {
            mSampleStep = (mSampleEnd - mSampleBegin) / TONE_SAMPLE_LEN;
        }
        else
        {
            mSampleStep = -((mSampleBegin - mSampleEnd) / TONE_SAMPLE_LEN);
        }

        ++mRead;
    }

    return mSampleBegin >> 8;
}

void Tone::update()
{
    if (!mHold && 0x10000 - mToneTimer <= mRate)
    {
        mFlags |= TONE_FLAGS_BEEP;
    }

    mToneTimer += mRate;
}

void Tone::setRate(uint16_t rate)
{
    mRate = rate;
}

void Tone::setPitch(uint16_t index)
{
    mNextIndex = index;
}

void Tone::setChirp(uint32_t chirp)
{
    mNextChirp = chirp;
}

void Tone::loadTable()
{
    uint8_t  val;
    uint16_t read;
    uint16_t size, i;

    read = mRead;

    size = read + TONE_BUFFER_LEN - mWrite;
    size = MIN(size, mLen);

    for (i = 0; i < size; ++i, --mLen)
    {
        val = mSineTable[mPhase >> 8];

        mPhase += mStep >> 16;
        mStep += mChirp;

        val = 128 - (128 >> mConfig.Tone_volume) + (val >> mConfig.Tone_volume);
        mBuffer[(mWrite + i) % TONE_BUFFER_LEN] = val;
    }

    mWrite += size;

    if (!mLen)
    {
        mFlags &= ~TONE_FLAGS_LOAD;
    }
}

void Tone::readFile(uint16_t size)
{
    qint64   br;
    uint16_t i;
    uint8_t  val;

    size = MIN(size, mWavSamples);
    br = mFile.read((char *) &mBuffer[mWrite % TONE_BUFFER_LEN], size);
    mWavSamples -= br;

    for (i = 0; i < br; ++i)
    {
        val = mBuffer[(mWrite + i) % TONE_BUFFER_LEN];
        val = 128 - (128 >> mConfig.Tone_sp_volume) + (val >> mConfig.Tone_sp_volume);
        mBuffer[(mWrite + i) % TONE_BUFFER_LEN] = val;
    }

    mWrite += br;

    if (mWavSamples == 0)
    {
        mFlags &= ~TONE_FLAGS_LOAD;
    }
}

void Tone::loadWAV()
{
    uint16_t read;
    uint16_t size;

    read = mRead;

    if (mWrite != read + TONE_BUFFER_LEN)
    {
        size = MIN(TONE_BUFFER_CHUNK, read + TONE_BUFFER_LEN - mWrite);

        if (mWrite / TONE_BUFFER_LEN != (mWrite + size) / TONE_BUFFER_LEN)
        {
            size -= TONE_BUFFER_LEN - (mWrite % TONE_BUFFER_LEN);
            readFile(TONE_BUFFER_LEN - (mWrite % TONE_BUFFER_LEN));
        }

        if (mFlags & TONE_FLAGS_LOAD)
        {
            readFile(size);
        }
    }
}

void Tone::load()
{
    switch (mMode)
    {
    case TONE_MODE_BEEP:
        loadTable();
        break;
    case TONE_MODE_WAV:
        loadWAV();
        break;
    }
}

void Tone::start(uint8_t mode)
{
    if (mState == TONE_STATE_IDLE)
    {
        mState = TONE_STATE_PLAY;

        mMode = mode;

        mFlags |= TONE_FLAGS_LOAD;

        mRead  = 0;
        mWrite = 0;

        load();

        mSampleActive = true;
    }
}

void Tone::stop()
{
    if (mState != TONE_STATE_IDLE)
    {
        mSampleActive = false;

        switch (mMode)
        {
        case TONE_MODE_BEEP:
            break;
        case TONE_MODE_WAV:
            mFile.close();
            break;
        }

        mState = TONE_STATE_IDLE;
    }

    mFlags &= ~TONE_FLAGS_STOP;
    mFlags &= ~TONE_FLAGS_LOAD;
}

void Tone::task()
{
    if (mFlags & TONE_FLAGS_BEEP)
    {
        if (mState == TONE_STATE_IDLE)
        {
            beep(mNextIndex, mNextChirp, TONE_LENGTH_125_MS);
        }

        mFlags &= ~TONE_FLAGS_BEEP;
    }

    if (mFlags & TONE_FLAGS_STOP)
    {
        stop();
    }

    if (mFlags & TONE_FLAGS_LOAD)
    {
        load();
    }
}

void Tone::beep(
        uint16_t index,
        uint32_t chirp,
        uint16_t len)
{
    if (mConfig.Tone_volume < 8)
    {
        stop();

        mStep  = ((int32_t) index * 3242 + 30212096) * TONE_SAMPLE_LEN;
        mChirp = chirp * TONE_SAMPLE_LEN * TONE_SAMPLE_LEN;
        mLen   = len / TONE_SAMPLE_LEN;

        start(TONE_MODE_BEEP);
    }
}

void Tone::play(QString filename)
{
    if (mConfig.Tone_sp_volume < 8)
    {
        stop();

        mFile.setFileName(mConfig.mAudioFolder + QString("/") + filename);

        if (mFile.open(QIODevice::ReadOnly))
        {
            mFile.seek(40);
            mFile.read((char *) &mWavSamples, sizeof(mWavSamples));

            mFile.seek(44);

            start(TONE_MODE_WAV);
        }
    }
}

void Tone::wait(void)
{
    while (mState != TONE_STATE_IDLE)
    {
        task();
    }
}

uint8_t Tone::canWrite(void)
{
    uint16_t c;

    c = mWrite - mRead;

    return (mState == TONE_STATE_IDLE) || (c > TONE_BUFFER_WRITE);
}

uint8_t Tone::isIdle(void)
{
    return mState == TONE_STATE_IDLE;
}

void Tone::hold(void)
{
    mHold = 1;
}

void Tone::release(void)
{
    mHold = 0;
}
