#include "SC_PlugIn.hpp"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;


#define SP_UPPER_BOUND 2139095040

#define SP_GET_INS_OUTS \
float *outIntAsFloat = out(0); \
float *outDec = out(1); \
float *outPlaying = out(2); \
float startIntAsFloat = in0(2); \
float startDec = in0(3); \
double startIntAsDouble = (double)(*reinterpret_cast<int32*>(&startIntAsFloat)); \
double startDecAsDouble = (double)startDec; \
double start = startIntAsDouble + startDecAsDouble; \
float endIntAsFloat = in0(4); \
float endDec = in0(5); \
double endIntAsDouble = (double)(*reinterpret_cast<int32*>(&endIntAsFloat)); \
double endDecAsDouble = (double)endDec; \
double end = endIntAsDouble + endDecAsDouble; \
float resetIntAsFloat = in0(6); \
float resetDec = in0(7); \
const int loop = (int)in0(8); \
float prevtrig = mPrevtrig; \
double pos = mPos; \
int playing = mPlaying; \

#define SPX_GET_INS_OUTS \
float *phase0IntAsFloat = out(0); \
float *phase0Dec = out(1); \
float *phase1IntAsFloat = out(2); \
float *phase1Dec = out(3); \
float *pan0 = out(4); \
float *phase2IntAsFloat = out(5); \
float *phase2Dec = out(6); \
float *phase3IntAsFloat = out(7); \
float *phase3Dec = out(8); \
float *pan1 = out(9); \
float *pan2 = out(10); \
float *outPlaying = out(11); \
float startIntAsFloat = in0(2); \
float startDec = in0(3); \
double startIntAsDouble = (double)(*reinterpret_cast<int32*>(&startIntAsFloat)); \
double startDecAsDouble = (double)startDec; \
double start = startIntAsDouble + startDecAsDouble; \
float endIntAsFloat = in0(4); \
float endDec = in0(5); \
double endIntAsDouble = (double)(*reinterpret_cast<int32*>(&endIntAsFloat)); \
double endDecAsDouble = (double)endDec; \
double end = endIntAsDouble + endDecAsDouble; \
float resetIntAsFloat = in0(6); \
float resetDec = in0(7); \
const int loop = (int)in0(8); \
int overlap = (int)in0(9); \
if (overlap > (end - start) * 0.5) { \
    overlap = (int)((end - start) * 0.5); \
} else if (overlap < 0) { \
    overlap = 0; \
} \
float prevtrig = mPrevtrig; \
double pos = mPos; \
int playing = mPlaying; \
int isOverlapping = mIsOverlapping; \
double oldPos = mOldPos; \
double overlapPos = mOverlapPos; \
int oldPlaying = mOldPlaying; \
int firstTime = mFirstTime; \


#define SP_TEST_TRIG \
if (prevtrig <= 0.f && trig > 0.f) { \
    double resetIntAsDouble = (double)(*reinterpret_cast<int32*>(&resetIntAsFloat)); \
    double resetDecAsDouble = (double)resetDec; \
    double reset = resetIntAsDouble + resetDecAsDouble; \
    if (reset < start) { \
        pos = start; \
    } else { \
        pos = reset; \
    } \
    playing = 0; \
} \

#define SPX_TEST_TRIG \
if (prevtrig <= 0.f && trig > 0.f) { \
    oldPos = pos; \
    oldPlaying = playing; \
    overlapPos = 0; \
    isOverlapping = 1; \
    double resetIntAsDouble = (double)(*reinterpret_cast<int32*>(&resetIntAsFloat)); \
    double resetDecAsDouble = (double)resetDec; \
    double reset = resetIntAsDouble + resetDecAsDouble; \
    if (reset < (start + overlap) && rate > 0) { \
        firstTime = 1; \
    } \
    if (reset < start) { \
        pos = start; \
    } else { \
        pos = reset; \
    } \
    playing = 0; \
} \

#define SP_WRITE_OUTS \
int32 posInt = (int32)pos; \
outIntAsFloat[i] = *reinterpret_cast<float*>(&posInt); \
outDec[i] = pos - posInt; \
outPlaying[i] = (playing == 0); \

#define SPX_WRITE_OUTS \
int32 posInt = (int32)pos; \
phase0IntAsFloat[i] = *reinterpret_cast<float*>(&posInt); \
phase0Dec[i] = pos - posInt; \
if (loop && pos < overlap && !firstTime) { \
    double endPos = pos + end - overlap; \
    int32 endPosInt = (int32)endPos; \
    phase1IntAsFloat[i] = *reinterpret_cast<float*>(&endPosInt); \
    phase1Dec[i] = endPos - endPosInt; \
    pan0[i] = 1 - (2 * pos / (overlap - 1)); \
} else { \
    phase1IntAsFloat[i] = 0; \
    phase1Dec[i] = 0; \
    pan0[i] = -1; \
} \
if (isOverlapping) { \
    int32 oldPosInt = (int32)oldPos; \
    phase2IntAsFloat[i] = *reinterpret_cast<float*>(&oldPosInt); \
    phase2Dec[i] = oldPos - oldPosInt; \
    if (loop && oldPos < overlap) { \
        double endOldPos = oldPos + end - overlap; \
        int32 endOldPosInt = (int32)endOldPos; \
        phase3IntAsFloat[i] = *reinterpret_cast<float*>(&endOldPosInt); \
        phase3Dec[i] = endOldPos - endOldPosInt; \
        pan1[i] = 1 - (2 * oldPos / (overlap - 1)); \
    } else { \
        phase3IntAsFloat[i] = 0; \
        phase3Dec[i] = 0; \
        pan1[i] = -1; \
    } \
    pan2[i] = 1 - (2 * overlapPos / (overlap - 1)); \
} else { \
    phase2IntAsFloat[i] = 0; \
    phase2Dec[i] = 0; \
    phase3IntAsFloat[i] = 0; \
    phase3Dec[i] = 0; \
    pan1[i] = -1; \
    pan2[i] = -1; \
} \
outPlaying[i] = (playing == 0); \

#define SP_INCREMENT_POS \
if (pos >= end) { \
    if (loop) { \
        while (pos >= end) { \
            pos = pos - end + start; \
        } \
        playing = 0; \
    } else { \
        pos = end; \
        playing = 1; \
    } \
} \
if (pos <= start) { \
    if (loop) { \
        while (pos < start) { \
            pos = pos - start + end; \
        } \
        playing = 0; \
    } else { \
        pos = start; \
        playing = -1; \
    } \
} \
if (playing == -1 && (rate > 0 || start < pos)) { \
    playing = 0; \
} \
if (playing == 1 && (rate < 0 || end > pos)) { \
    playing = 0; \
} \
if (playing == 0) { \
    pos += rate; \
} \

#define SPX_INCREMENT_POS \
if (loop) { \
    playing = 0; \
} \
if (playing == -1 && (rate > 0 || start < pos)) { \
    playing = 0; \
} \
if (playing == 1 && (rate < 0 || end > pos)) { \
    playing = 0; \
} \
if (playing == 0) { \
    pos += rate; \
} \
if (loop) { \
    while (pos >= (end - overlap)) { \
        pos = pos - (end - overlap) + start; \
    } \
    while (pos < start) { \
        pos = pos - start + (end - overlap); \
    } \
} else { \
    if (pos >= end) { \
        pos = end; \
        playing = 1; \
    } else if (pos <= start) { \
        pos = start; \
        playing = -1; \
    } \
} \
if (firstTime && pos > overlap) { \
    firstTime = 0; \
} \
if (isOverlapping) { \
    if (oldPlaying == 0) { \
        oldPos += rate; \
    } \
    overlapPos += 1; \
    if (overlapPos >= overlap) { \
        isOverlapping = 0; \
    } \
    if (loop) { \
        while (oldPos >= (end - overlap)) { \
            oldPos = oldPos - (end - overlap) + start; \
        } \
        while (oldPos < start) { \
            oldPos = oldPos - start + (end - overlap); \
        } \
    } else { \
        if (oldPos >= end) { \
            oldPos = end; \
            oldPlaying = 1; \
        } else if (oldPos <= start) { \
            oldPos = start; \
            oldPlaying = -1; \
        } \
    } \
} \

#define SP_STORE_STRUCT \
mPrevtrig = trig; \
mPos = pos; \
mPlaying = playing; \

#define SPX_STORE_STRUCT \
mPrevtrig = trig; \
mPos = pos; \
mPlaying = playing; \
mIsOverlapping = isOverlapping; \
mOldPos = oldPos; \
mOverlapPos = overlapPos; \
mOldPlaying = oldPlaying; \
mFirstTime = firstTime; \


//////////////////////////////////////////////////////////////////
// SUPERPHASOR
//////////////////////////////////////////////////////////////////

struct SuperPhasor : public SCUnit{

public:
    SuperPhasor() {
        // 1. initialize the unit generator state variables.
        mPrevtrig = in0(0);
        float startIntAsFloat = in0(2);
        float startDec = in0(3);
        double startIntAsDouble = (double)(*reinterpret_cast<int32*>(&startIntAsFloat));
        double startDecAsDouble = (double)startDec;
        double start = startIntAsDouble + startDecAsDouble;
        float resetIntAsFloat = in0(6);
        float resetDec = in0(7);
        double resetIntAsDouble = (double)(*reinterpret_cast<int32*>(&resetIntAsFloat));
        double resetDecAsDouble = (double)resetDec;
        double reset = resetIntAsDouble + resetDecAsDouble;
        if (reset < start) {
            mPos = start;
        } else {
            mPos = reset;
        }
        mPlaying = 0;

        // 2. set calc function
        if (isAudioRateIn(0)) {
            if (isAudioRateIn(1)) {
                // both trig and rate are audio rate
                set_calc_function<SuperPhasor,&SuperPhasor::next_aa>();
            } else {
                // only trig is audio rate
                set_calc_function<SuperPhasor,&SuperPhasor::next_ak>();
            }

        } else {
            if (isAudioRateIn(1)) {
                // only rate is audio rate
                set_calc_function<SuperPhasor,&SuperPhasor::next_ka>();
            } else {
                // nothing is audio rate
                set_calc_function<SuperPhasor,&SuperPhasor::next_kk>();
            }
        }
    }

private:
    float mPrevtrig;
    double mPos;
    int mPlaying;



    //////////////////////////////////////////////////////////////////

    // calculation function for all control rate arguments
    void next_kk(int inNumSamples)
    {
        SP_GET_INS_OUTS

        const float trig = in0(0);
        const float rate = in0(1);

        SP_TEST_TRIG

        for (int i=0; i < inNumSamples; ++i)
        {
            SP_WRITE_OUTS
            SP_INCREMENT_POS
        }

        SP_STORE_STRUCT
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate rate
    void next_ka(int inNumSamples)
    {
        SP_GET_INS_OUTS

        const float trig = in0(0);
        const float* rateBlock = in(1);
        float rate;

        SP_TEST_TRIG

        for (int i=0; i < inNumSamples; ++i)
        {
            rate = rateBlock[i];
            SP_WRITE_OUTS
            SP_INCREMENT_POS
        }

        SP_STORE_STRUCT
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate trig
    void next_ak(int inNumSamples)
    {
        SP_GET_INS_OUTS

        const float* trigBlock = in(0);
        const float rate = in0(1);
        float trig;

        for (int i=0; i < inNumSamples; ++i)
        {
            trig = trigBlock[i];
            SP_TEST_TRIG
            SP_WRITE_OUTS
            SP_INCREMENT_POS
            prevtrig = trig;
        }

        SP_STORE_STRUCT
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate trig and rate
    void next_aa(int inNumSamples)
    {
        SP_GET_INS_OUTS

        const float* trigBlock = in(0);
        const float* rateBlock = in(1);
        float trig;
        float rate;

        for (int i=0; i < inNumSamples; ++i)
        {
            trig = trigBlock[i];
            rate = rateBlock[i];
            SP_TEST_TRIG
            SP_WRITE_OUTS
            SP_INCREMENT_POS
            prevtrig = trig;
        }

        SP_STORE_STRUCT
    }
};



//////////////////////////////////////////////////////////////////
// SUPERPHASORX
//////////////////////////////////////////////////////////////////

struct SuperPhasorX : public SCUnit{

public:
    SuperPhasorX() {
        // 1. initialize the unit generator state variables.
        mPrevtrig = in0(0);
        float rate = in0(1);
        float startIntAsFloat = in0(2);
        float startDec = in0(3);
        double startIntAsDouble = (double)(*reinterpret_cast<int32*>(&startIntAsFloat));
        double startDecAsDouble = (double)startDec;
        double start = startIntAsDouble + startDecAsDouble;
        float resetIntAsFloat = in0(6);
        float resetDec = in0(7);
        double resetIntAsDouble = (double)(*reinterpret_cast<int32*>(&resetIntAsFloat));
        double resetDecAsDouble = (double)resetDec;
        double reset = resetIntAsDouble + resetDecAsDouble;
        int overlap = (int)in0(9);
        if (reset < start) {
            mPos = start;
        } else {
            mPos = reset;
        }
        mPlaying = 0;
        mIsOverlapping = 0;
        mOldPos = 0;
        mOverlapPos = 0;
        mOldPlaying = 0;
        if (reset < (start + overlap) && rate > 0) {
            mFirstTime = 1;
        } else {
            mFirstTime = 0;
        }

        // 2. set the calculation function.
        if (isAudioRateIn(0)) {
            if (isAudioRateIn(1)) {
                // both trig and rate are audio rate
                set_calc_function<SuperPhasorX,&SuperPhasorX::next_aa>();
            } else {
                // only trig is audio rate
                set_calc_function<SuperPhasorX,&SuperPhasorX::next_ak>();
            }

        } else {
            if (isAudioRateIn(1)) {
                // only rate is audio rate
                set_calc_function<SuperPhasorX,&SuperPhasorX::next_ka>();
            } else {
                // nothing is audio rate
                set_calc_function<SuperPhasorX,&SuperPhasorX::next_kk>();
            }
        }
    }

private:
    float mPrevtrig;
    double mPos;
    int mPlaying;
    int mIsOverlapping;
    double mOldPos;
    double mOverlapPos;
    int mOldPlaying;
    int mFirstTime;



    //////////////////////////////////////////////////////////////////

    // calculation function for all control rate arguments
    void next_kk(int inNumSamples)
    {
        SPX_GET_INS_OUTS

        const float trig = in0(0);
        const float rate = in0(1);

        SPX_TEST_TRIG

        for (int i=0; i < inNumSamples; ++i)
        {
            SPX_WRITE_OUTS
            SPX_INCREMENT_POS
        }

        SPX_STORE_STRUCT
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate rate
    void next_ka(int inNumSamples)
    {
        SPX_GET_INS_OUTS

        const float trig = in0(0);
        const float* rateBlock = in(1);
        float rate = in0(1);

        SPX_TEST_TRIG

        for (int i=0; i < inNumSamples; ++i)
        {
            rate = rateBlock[i];
            SPX_WRITE_OUTS
            SPX_INCREMENT_POS
        }

        SPX_STORE_STRUCT
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate trig
    void next_ak(int inNumSamples)
    {
        SPX_GET_INS_OUTS

        const float* trigBlock = in(0);
        const float rate = in0(1);
        float trig;

        for (int i=0; i < inNumSamples; ++i)
        {
            trig = trigBlock[i];
            SPX_TEST_TRIG
            SPX_WRITE_OUTS
            SPX_INCREMENT_POS
            prevtrig = trig;
        }

        SPX_STORE_STRUCT
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate trig and rate
    void next_aa(int inNumSamples)
    {
        SPX_GET_INS_OUTS

        const float* trigBlock = in(0);
        const float* rateBlock = in(1);
        float trig;
        float rate = in0(1);

        for (int i=0; i < inNumSamples; ++i)
        {
            trig = trigBlock[i];
            rate = rateBlock[i];
            SPX_TEST_TRIG
            SPX_WRITE_OUTS
            SPX_INCREMENT_POS
            prevtrig = trig;
        }

        SPX_STORE_STRUCT
    }
};




//////////////////////////////////////////////////////////////////
// SUPERBUFRD
//////////////////////////////////////////////////////////////////


inline double sc_loop(Unit *unit, double in, double hi, int loop)
{
	// avoid the divide if possible
	if (in >= hi) {
		if (!loop) {
			unit->mDone = true;
			return hi;
		}
		in -= hi;
		if (in < hi) return in;
	} else if (in < 0.) {
		if (!loop) {
			unit->mDone = true;
			return 0.;
		}
		in += hi;
		if (in >= 0.) return in;
	} else return in;

	return in - hi * floor(in/hi);
}

#define LOOP_INNER_BODY_1(SAMPLE_INDEX) \
	OUT(channel)[SAMPLE_INDEX] = table1[index]; \

#define LOOP_INNER_BODY_2(SAMPLE_INDEX) \
	float b = table1[index]; \
	float c = table2[index]; \
	OUT(channel)[SAMPLE_INDEX] = b + fracphase * (c - b); \

#define LOOP_INNER_BODY_4(SAMPLE_INDEX) \
	float a = table0[index]; \
	float b = table1[index]; \
	float c = table2[index]; \
	float d = table3[index]; \
	OUT(channel)[SAMPLE_INDEX] = cubicinterp(fracphase, a, b, c, d); \


#define LOOP_BODY_4(SAMPLE_INDEX) \
		phase = sc_loop((Unit*)unit, phase, loopMax, loop); \
		int32 iphase = (int32)phase; \
		const float* table1 = bufData + iphase * bufChannels; \
		const float* table0 = table1 - bufChannels; \
		const float* table2 = table1 + bufChannels; \
		const float* table3 = table2 + bufChannels; \
		if (iphase == 0) { \
			if (loop) { \
				table0 += bufSamples; \
			} else { \
				table0 += bufChannels; \
			} \
		} else if (iphase >= guardFrame) { \
			if (iphase == guardFrame) { \
				if (loop) { \
					table3 -= bufSamples; \
				} else { \
					table3 -= bufChannels; \
				} \
			} else { \
				if (loop) { \
					table2 -= bufSamples; \
					table3 -= bufSamples; \
				} else { \
					table2 -= bufChannels; \
					table3 -= 2 * bufChannels; \
				} \
			} \
		} \
		int32 index = 0; \
		float fracphase = phase - (double)iphase; \
		if(numOutputs == bufChannels) { \
			for (uint32 channel=0; channel<numOutputs; ++channel) { \
				LOOP_INNER_BODY_4(SAMPLE_INDEX) \
				index++; \
			} \
		} else if (numOutputs < bufChannels) { \
			for (uint32 channel=0; channel<numOutputs; ++channel) { \
				LOOP_INNER_BODY_4(SAMPLE_INDEX) \
				index++; \
			} \
			index += (bufChannels - numOutputs); \
		} else { \
			for (uint32 channel=0; channel<bufChannels; ++channel) { \
				LOOP_INNER_BODY_4(SAMPLE_INDEX) \
				index++; \
			} \
			for (uint32 channel=bufChannels; channel<numOutputs; ++channel) { \
				OUT(channel)[SAMPLE_INDEX] = 0.f; \
				index++; \
			} \
		} \


#define LOOP_BODY_2(SAMPLE_INDEX) \
		phase = sc_loop((Unit*)unit, phase, loopMax, loop); \
		int32 iphase = (int32)phase; \
		const float* table1 = bufData + iphase * bufChannels; \
		const float* table2 = table1 + bufChannels; \
		if (iphase > guardFrame) { \
			if (loop) { \
				table2 -= bufSamples; \
			} else { \
				table2 -= bufChannels; \
			} \
		} \
		int32 index = 0; \
		float fracphase = phase - (double)iphase; \
		if(numOutputs == bufChannels) { \
			for (uint32 channel=0; channel<numOutputs; ++channel) { \
				LOOP_INNER_BODY_2(SAMPLE_INDEX) \
				index++; \
			} \
		} else if (numOutputs < bufChannels) { \
			for (uint32 channel=0; channel<numOutputs; ++channel) { \
				LOOP_INNER_BODY_2(SAMPLE_INDEX) \
				index++; \
			} \
			index += (bufChannels - numOutputs); \
		} else { \
			for (uint32 channel=0; channel<bufChannels; ++channel) { \
				LOOP_INNER_BODY_2(SAMPLE_INDEX) \
				index++; \
			} \
			for (uint32 channel=bufChannels; channel<numOutputs; ++channel) { \
				OUT(channel)[SAMPLE_INDEX] = 0.f; \
				index++; \
			} \
		} \


#define LOOP_BODY_1(SAMPLE_INDEX) \
		phase = sc_loop((Unit*)unit, phase, loopMax, loop); \
		int32 iphase = (int32)phase; \
		const float* table1 = bufData + iphase * bufChannels; \
		int32 index = 0; \
		if(numOutputs == bufChannels) { \
			for (uint32 channel=0; channel<numOutputs; ++channel) { \
				LOOP_INNER_BODY_1(SAMPLE_INDEX) \
				index++; \
			} \
		} else if (numOutputs < bufChannels) { \
			for (uint32 channel=0; channel<numOutputs; ++channel) { \
				LOOP_INNER_BODY_1(SAMPLE_INDEX) \
				index++; \
			} \
			index += (bufChannels - numOutputs); \
		} else { \
			for (uint32 channel=0; channel<bufChannels; ++channel) { \
				LOOP_INNER_BODY_1(SAMPLE_INDEX) \
				index++; \
			} \
			for (uint32 channel=bufChannels; channel<numOutputs; ++channel) { \
				OUT(channel)[SAMPLE_INDEX] = 0.f; \
				index++; \
			} \
		} \



#define CHECK_BUFFER_DATA \
if (!bufData) { \
	if(unit->mWorld->mVerbosity > -1 && !unit->mDone && (unit->m_failedBufNum != fbufnum)) { \
		Print("Buffer UGen: no buffer data\n"); \
		unit->m_failedBufNum = fbufnum; \
	} \
	ClearUnitOutputs(unit, inNumSamples); \
	return; \
} else { \
	if (bufChannels != numOutputs) { \
		if(unit->mWorld->mVerbosity > -1 && !unit->mDone && (unit->m_failedBufNum != fbufnum)) { \
			Print("Buffer UGen channel mismatch: expected %i, yet buffer has %i channels\n", \
				  numOutputs, bufChannels); \
			unit->m_failedBufNum = fbufnum; \
			} \
		} \
} \

struct SuperBufRd : public Unit
{
	float m_fbufnum;
	float m_failedBufNum;
	SndBuf *m_buf;
};

void SuperBufRd_Ctor(SuperBufRd *unit);
void SuperBufRd_next_4(SuperBufRd *unit, int inNumSamples);
void SuperBufRd_next_2(SuperBufRd *unit, int inNumSamples);
void SuperBufRd_next_1(SuperBufRd *unit, int inNumSamples);

void SuperBufRd_Ctor(SuperBufRd *unit)
{
	int interp = (int)ZIN0(4);
	switch (interp) {
		case 1 : SETCALC(SuperBufRd_next_1); break;
		case 2 : SETCALC(SuperBufRd_next_2); break;
		default : SETCALC(SuperBufRd_next_4); break;
	}

	unit->m_fbufnum = -1e9f;
	unit->m_failedBufNum = -1e9f;

	SuperBufRd_next_1(unit, 1);
}

void SuperBufRd_next_4(SuperBufRd *unit, int inNumSamples)
{
    float *phaseInt = ZIN(1);
    float *phaseDec = ZIN(2);
	int32 loop     = (int32)ZIN0(3);

	GET_BUF_SHARED
	uint32 numOutputs = unit->mNumOutputs;

	CHECK_BUFFER_DATA

	double loopMax = (double)(loop ? bufFrames : bufFrames - 1);

	for (int i=0; i<inNumSamples; ++i) {
        float phaseIntIn = ZXP(phaseInt);
        double phaseDecDub = ZXP(phaseDec);
        double phaseIntDub = (double)(*reinterpret_cast<int32*>(&phaseIntIn));
        double phase = phaseIntDub + phaseDecDub;
		LOOP_BODY_4(i)
	}
}

void SuperBufRd_next_2(SuperBufRd *unit, int inNumSamples)
{
    float *phaseInt = ZIN(1);
    float *phaseDec = ZIN(2);
	int32 loop     = (int32)ZIN0(3);

	GET_BUF_SHARED
	uint32 numOutputs = unit->mNumOutputs;

	CHECK_BUFFER_DATA

	double loopMax = (double)(loop ? bufFrames : bufFrames - 1);

	for (int i=0; i<inNumSamples; ++i) {
        float phaseIntIn = ZXP(phaseInt);
        double phaseDecDub = ZXP(phaseDec);
        double phaseIntDub = (double)(*reinterpret_cast<int32*>(&phaseIntIn));
        double phase = phaseIntDub + phaseDecDub;
		LOOP_BODY_2(i)
	}
}

void SuperBufRd_next_1(SuperBufRd *unit, int inNumSamples)
{
    float *phaseInt = ZIN(1);
    float *phaseDec = ZIN(2);
	int32 loop     = (int32)ZIN0(3);

	GET_BUF_SHARED
	uint32 numOutputs = unit->mNumOutputs;

	CHECK_BUFFER_DATA

	double loopMax = (double)(loop ? bufFrames : bufFrames - 1);

	for (int i=0; i<inNumSamples; ++i) {
        float phaseIntIn = ZXP(phaseInt);
        double phaseDecDub = ZXP(phaseDec);
        double phaseIntDub = (double)(*reinterpret_cast<int32*>(&phaseIntIn));
        double phase = phaseIntDub + phaseDecDub;
		LOOP_BODY_1(i)
	}
}



//////////////////////////////////////////////////////////////////
// SUPERBUFFRAMES
//////////////////////////////////////////////////////////////////

#define CTOR_GET_BUF \
	float fbufnum  = ZIN0(0); \
	fbufnum = sc_max(0.f, fbufnum); \
	uint32 bufnum = (int)fbufnum; \
	World *world = unit->mWorld; \
	SndBuf *buf; \
	if (bufnum >= world->mNumSndBufs) { \
		int localBufNum = bufnum - world->mNumSndBufs; \
		Graph *parent = unit->mParent; \
		if(localBufNum <= parent->localBufNum) { \
			buf = parent->mLocalSndBufs + localBufNum; \
		} else { \
			bufnum = 0; \
			buf = world->mSndBufs + bufnum; \
		} \
	} else { \
		buf = world->mSndBufs + bufnum; \
	}

struct BufInfoUnit : public Unit
{
	float m_fbufnum;
	SndBuf *m_buf;
};

void SuperBufFrames_next(BufInfoUnit *unit, int inNumSamples);
void SuperBufFrames_Ctor(BufInfoUnit *unit, int inNumSamples);

void SuperBufFrames_next(BufInfoUnit *unit, int inNumSamples)
{
	SIMPLE_GET_BUF_SHARED
    int32 framesInt = (int32)buf->frames;
    ZOUT0(0) = *reinterpret_cast<float*>(&framesInt);
}

void SuperBufFrames_Ctor(BufInfoUnit *unit, int inNumSamples)
{
	SETCALC(SuperBufFrames_next);
	CTOR_GET_BUF
	unit->m_fbufnum = fbufnum;
	unit->m_buf = buf;
    int32 framesInt = (int32)buf->frames;
    ZOUT0(0) = *reinterpret_cast<float*>(&framesInt);
}


// the entry point is called by the host when the plug-in is loaded
PluginLoad(SuperBufRdUGens)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable; // store pointer to InterfaceTable

    // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
    // destructor function.
    // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
    registerUnit<SuperPhasor>(ft, "SuperPhasor");
    registerUnit<SuperPhasorX>(ft, "SuperPhasorX");

    DefineSimpleUnit(SuperBufRd);

#define DefineBufInfoUnit(name) \
	(*ft->fDefineUnit)(#name, sizeof(BufInfoUnit), (UnitCtorFunc)&name##_Ctor, 0, 0);

    DefineBufInfoUnit(SuperBufFrames);
}
