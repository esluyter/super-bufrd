#include "SC_PlugIn.hpp"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;


//////////////////////////////////////////////////////////////////
// SUPERPOLL
//////////////////////////////////////////////////////////////////

struct SuperPoll : public SCUnit{

public:
    SuperPoll()
    {
        // 1. initialize the unit generator state variables.
        m_trig = in0(0);
    	const int idSize = (int)in0(4); // number of chars in the id string
    	m_id_string = (char*)RTAlloc(mWorld, (idSize + 1) * sizeof(char));

    	if (!m_id_string) {
    		Print("SuperPoll: RT memory allocation failed\n");
    		set_calc_function<SuperPoll,&SuperPoll::next_nop>();
    		return;
    	}

    	for(int i = 0; i < idSize; i++)
    	   m_id_string[i] = (char)in0(5+i);

    	m_id_string[idSize] = '\0';
    	m_mayprint = mWorld->mVerbosity >= -1;

        // 2. set calc function
        if (isAudioRateIn(0)) {
    		if (isAudioRateIn(1)) {
                if (isAudioRateIn(2)) {
                    set_calc_function<SuperPoll,&SuperPoll::next_aaa>();
                } else {
        			set_calc_function<SuperPoll,&SuperPoll::next_aak>();
                }
    		} else {
    			set_calc_function<SuperPoll,&SuperPoll::next_ak>();
    		}
    	} else {
    		set_calc_function<SuperPoll,&SuperPoll::next_kk>();
    	}

    }

    ~SuperPoll()
    {
        RTFree(mWorld, m_id_string);
    }

private:
    int m_samplesRemain, m_intervalSamples;
	float m_trig;
	float m_lastPoll;
	char *m_id_string;
	bool m_mayprint;
    //////////////////////////////////////////////////////////////////

    void next_nop(int inNumSamples)
    {

    }

    void next_aaa(int inNumSamples)
    {
        const float *trig = in(0);
        const float *msdBlock = in(1);
        const float *lsdBlock = in(2);
        float lasttrig = m_trig;
        /*
        float *msdOut = out(0);
        float *lsdOut = out(1);
        */
        for (int i=0; i < inNumSamples; ++i)
        {
            double msd = msdBlock[i];
            double lsd = lsdBlock[i];
            double value = msd + lsd;
            /*

            float valueMsd = (float)value;
            float valueLsd = (float)(value - valueMsd);
            msdOut[i] = valueMsd;
            lsdOut[i] = valueLsd;

            Print("%f\n", trig[i]);
            */

            if((lasttrig <= 0.0) && (trig[i] > 0.0)){
    			if (m_mayprint)
    				Print("%s: %f\n", m_id_string, value);

                /*
    			if (in0(3) >= 0.0)
    				SendTrigger(&mParent->mNode, (int)in0(3), msd);
                    */
    		}
    		lasttrig = trig[i];
    	}
    	m_trig = lasttrig;

    }

    void next_aak(int inNumSamples)
    {
        const float *trig = in(0);
        const float *msdBlock = in(1);
        double lsd = in0(2);
        float lasttrig = m_trig;

        for (int i=0; i < inNumSamples; ++i)
        {
            double msd = msdBlock[i];
            double value = msd + lsd;

            if((lasttrig <= 0.0) && (trig[i] > 0.0)){
    			if (m_mayprint)
    				Print("%s: %f\n", m_id_string, value);

                /*
    			if (in0(3) >= 0.0)
    				SendTrigger(&mParent->mNode, (int)in0(3), msd);
                    */
    		}
    		lasttrig = trig[i];
    	}
    	m_trig = lasttrig;
    }

    void next_ak(int inNumSamples)
    {
        const float *trig = in(0);
        double msd = in0(1);
        double lsd = in0(2);
        double value = msd + lsd;
        float lasttrig = m_trig;

        for (int i=0; i < inNumSamples; ++i)
        {
            if((lasttrig <= 0.0) && (trig[i] > 0.0)){
    			if (m_mayprint)
    				Print("%s: %f\n", m_id_string, value);

                /*
    			if (in0(3) >= 0.0)
    				SendTrigger(&mParent->mNode, (int)in0(3), msd);
                    */
    		}
    		lasttrig = trig[i];
    	}
    	m_trig = lasttrig;
    }

    void next_kk(int inNumSamples)
    {
        const float trig = in0(0);
        double msd = in0(1);
        double lsd = in0(2);
        double value = msd + lsd;

        if((m_trig <= 0.0) && (trig > 0.0)){
			if (m_mayprint)
				Print("%s: %f\n", m_id_string, value);

            /*
			if (in0(3) >= 0.0)
				SendTrigger(&mParent->mNode, (int)in0(3), msd);
                */
		}
    	m_trig = trig;
    }
};




// the entry point is called by the host when the plug-in is loaded
PluginLoad(SuperBufRdUGens)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable; // store pointer to InterfaceTable

    // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
    // destructor function.
    // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
    registerUnit<SuperPoll>(ft, "SuperPoll");
}
