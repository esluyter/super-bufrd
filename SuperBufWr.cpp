#include "SC_PlugIn.hpp"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;

struct SuperBufWr : public Unit {
    float m_fbufnum;
    SndBuf* m_buf;
};

void SuperBufWr_Ctor(SuperBufWr* unit);
void SuperBufWr_next(SuperBufWr* unit, int inNumSamples);

static inline bool checkBuffer(Unit* unit, const float* bufData, uint32 bufChannels, uint32 expectedChannels,
                               int inNumSamples) {
    if (!bufData)
        goto handle_failure;

    if (expectedChannels > bufChannels) {
        if (unit->mWorld->mVerbosity > -1 && !unit->mDone)
            Print("Buffer UGen channel mismatch: expected %i, yet buffer has %i channels\n", expectedChannels,
                  bufChannels);
        goto handle_failure;
    }
    return true;

handle_failure:
    unit->mDone = true;
    ClearUnitOutputs(unit, inNumSamples);
    return false;
}

inline double sc_loop(Unit* unit, double in, double hi, int loop) {
    // avoid the divide if possible
    if (in >= hi) {
        if (!loop) {
            unit->mDone = true;
            return hi;
        }
        in -= hi;
        if (in < hi)
            return in;
    } else if (in < 0.) {
        if (!loop) {
            unit->mDone = true;
            return 0.;
        }
        in += hi;
        if (in >= 0.)
            return in;
    } else
        return in;

    return in - hi * floor(in / hi);
}


void SuperBufWr_Ctor(SuperBufWr* unit) {
    SETCALC(SuperBufWr_next);
    unit->m_fbufnum = -1e9f;
    ClearUnitOutputs(unit, 1);
}

void SuperBufWr_next(SuperBufWr* unit, int inNumSamples) {
    float* phaseIn_msd = ZIN(1);
    float* phaseIn_lsd = ZIN(2);
    int32 loop = (int32)ZIN0(3);

    GET_BUF
    uint32 numInputChannels = unit->mNumInputs - 4;
    if (!checkBuffer(unit, bufData, bufChannels, numInputChannels, inNumSamples))
        return;

    double loopMax = (double)(bufFrames - (loop ? 0 : 1));

    for (int32 k = 0; k < inNumSamples; ++k) {
        double phaseIn = static_cast<double>(ZXP(phaseIn_msd)) + static_cast<double>(ZXP(phaseIn_lsd));
        double phase = sc_loop((Unit*)unit, phaseIn, loopMax, loop);
        int32 iphase = (int32)phase;
        float* table0 = bufData + iphase * bufChannels;
        for (uint32 channel = 0; channel < numInputChannels; ++channel)
          table0[channel] = IN(channel+4)[k];
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
    DefineSimpleUnit(SuperBufWr);
}
