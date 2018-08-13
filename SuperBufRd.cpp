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

#define SP_WRITE_OUTS \
int32 posInt = (int32)pos; \
outIntAsFloat[i] = *reinterpret_cast<float*>(&posInt); \
outDec[i] = pos - posInt; \
outPlaying[i] = (playing == 0); \

#define SP_INCREMENT_POS \
if (playing == -1 && rate > 0) { \
    playing = 0; \
} \
if (playing == 1 && rate < 0) { \
    playing = 0; \
} \
if (playing == 0) { \
    pos += rate; \
} \
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

#define SP_STORE_STRUCT \
mPrevtrig = trig; \
mPos = pos; \
mPlaying = playing; \


//////////////////////////////////////////////////////////////////
// SUPERPHASOR
//////////////////////////////////////////////////////////////////

struct SuperPhasor : public SCUnit{

public:
    SuperPhasor() {
        // 1. set the calculation function.
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

        // 2. initialize the unit generator state variables.
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

        // 3. calculate one sample of output.
        next_kk(1);
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

// the entry point is called by the host when the plug-in is loaded
PluginLoad(SuperBufRdUGens)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable; // store pointer to InterfaceTable

    // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
    // destructor function.
    // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
    registerUnit<SuperPhasor>(ft, "SuperPhasor");
    //registerUnit<SuperBufRd>(ft, "SuperBufRd");

    DefineSimpleUnit(SuperBufRd);
}
