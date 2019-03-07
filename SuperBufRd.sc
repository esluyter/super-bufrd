SuperPhasor : MultiOutUGen {
    *ar { arg trig=0, rate=1, start=0, end=48000, reset=0, loop=1;
        ^this.multiNew('audio', trig, rate, start, end, reset, loop, false);
    }
    *arDetails { arg trig=0, rate=1, start=0, end=48000, reset=0, loop=1;
        ^this.multiNew('audio', trig, rate, start, end, reset, loop, true);
    }
    *new1 { arg ugen_rate, trig, rate, start, end, reset, loop, details;
        var msd, lsd, isPlaying;
        start = start.asPair.asArray;
        end = end.asPair.asArray;
        reset = reset.asPair.asArray;
        # msd, lsd, isPlaying = super.new.rate_(ugen_rate).addToSynth.init(([trig, rate] ++ start ++ end ++ reset ++ [loop]));
        if (details) {
            ^[SuperPair(msd, lsd), isPlaying];
        } {
            ^SuperPair(msd, lsd);
        };
    }
    init { arg theInputs;
        inputs = theInputs;
        ^this.initOutputs(3, rate);
    }
}

SuperPhasorX : MultiOutUGen {
    *ar { arg trig=0, rate=1, start=0, end=48000, reset=0, loop=1, overlap=5;
        ^this.multiNew('audio', trig, rate, start, end, reset, loop, overlap)
    }
    *new1 { arg ugen_rate, trig, rate, start, end, reset, loop, overlap;
        var msd0, lsd0, msd1, lsd1, pan0, msd2, lsd2, msd3, lsd3, pan1, pan2, isPlaying;
        start = start.asPair.asArray;
        end = end.asPair.asArray;
        reset = reset.asPair.asArray;
        # msd0, lsd0, msd1, lsd1, pan0, msd2, lsd2, msd3, lsd3, pan1, pan2, isPlaying = super.new.rate_(ugen_rate).addToSynth.init(([trig, rate] ++ start ++ end ++ reset ++ [loop, overlap]));

        ^[SuperPair(msd0, lsd0), SuperPair(msd1, lsd1), pan0, SuperPair(msd2, lsd2), SuperPair(msd3, lsd3), pan1, pan2, isPlaying];
    }
    init { arg theInputs;
        inputs = theInputs;
        ^this.initOutputs(12, rate);
    }
}

SuperBufRd : MultiOutUGen {
    *ar { arg numChannels=1, bufnum=0, phase=0, loop=1, quality=2;
        ^this.multiNew('audio', numChannels, bufnum, phase, loop, quality)
    }
    *new1 { arg rate, numChannels, bufnum, phase, loop, quality;
        phase = phase.asPair.asArray;
        ^super.new.rate_(rate).addToSynth.init(numChannels, [bufnum] ++ phase ++ [loop, quality]);
    }

    init { arg argNumChannels, theInputs;
		inputs = theInputs;
		^this.initOutputs(argNumChannels, rate);
	}
    argNamesInputsOffset { ^2 }
	checkInputs {
		if (rate == 'audio' and: {inputs.at(1).rate != 'audio'}, {
			^("phaseInt input is not audio rate: " + inputs.at(1) + inputs.at(1).rate);
		});
        if (rate == 'audio' and: {inputs.at(2).rate != 'audio'}, {
			^("phaseDec input is not audio rate: " + inputs.at(2) + inputs.at(2).rate);
		});
		^this.checkValidInputs
	}
}

SuperBufFrames : MultiOutUGen {
    *kr { arg bufnum;
		^this.multiNew('control', bufnum)
	}

	// the .ir method is not the safest choice. Since a buffer can be reallocated at any time,
	// using .ir will not track the changes.
	*ir { arg bufnum;
		^this.multiNew('scalar',bufnum)
	}

    *new1 { arg rate, bufnum;
        var msd, lsd;
        # msd, lsd = super.new.rate_(rate).addToSynth.init(bufnum);
        ^SuperPair(msd, lsd);
    }

    init { arg ... theInputs;
        inputs = theInputs;
        ^this.initOutputs(2, rate);
    }
}


SuperPlayBuf {
    *ar { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2;
        var pos;
        end = end ? SuperBufFrames.kr(bufnum);
        rate = BufRateScale.kr(bufnum) * rate;
        pos = SuperPhasor.ar(trig, rate, start, end, reset, loop);
        ^SuperBufRd.ar(numChannels, bufnum, pos, 0, quality);
    }

    *arDetails { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2;
        var pos, isPlaying;
        end = end ? SuperBufFrames.kr(bufnum);
        rate = BufRateScale.kr(bufnum) * rate;
        # pos, isPlaying = SuperPhasor.arDetails(trig, rate, start, end, reset, loop);
        ^[SuperBufRd.ar(numChannels, bufnum, pos, 0, quality), pos, isPlaying];
    }
}

SuperPlayBufX {
    *ar { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2, fadeTime=0.01;
        var phase0, phase1, pan0, phase2, phase3, pan1, pan2, playing;
        var sig0, sig1, sig2, sig3, sig;

        end = end ? SuperBufFrames.kr(bufnum);
        rate = BufRateScale.kr(bufnum) * rate;
        # phase0, phase1, pan0, phase2, phase3, pan1, pan2, playing = SuperPhasorX.ar(trig, rate, start, end, reset, loop, fadeTime * SampleRate.ir);
        sig0 = SuperBufRd.ar(numChannels, bufnum, phase0, 0, quality);
        sig1 = SuperBufRd.ar(numChannels, bufnum, phase1, 0, quality);
        sig2 = SuperBufRd.ar(numChannels, bufnum, phase2, 0, quality);
        sig3 = SuperBufRd.ar(numChannels, bufnum, phase3, 0, quality);
        ^XFade2.ar(XFade2.ar(sig0, sig1, pan0), XFade2.ar(sig2, sig3, pan1), pan2);
    }

    *arDetails { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2, fadeTime=0.01;
        var phase0, phase1, pan0, phase2, phase3, pan1, pan2, playing;
        var sig0, sig1, sig2, sig3, sig;

        end = end ? SuperBufFrames.kr(bufnum);
        rate = BufRateScale.kr(bufnum) * rate;
        # phase0, phase1, pan0, phase2, phase3, pan1, pan2, playing = SuperPhasorX.ar(trig, rate, start, end, reset, loop, fadeTime * SampleRate.ir);
        sig0 = SuperBufRd.ar(numChannels, bufnum, phase0, 0, quality);
        sig1 = SuperBufRd.ar(numChannels, bufnum, phase1, 0, quality);
        sig2 = SuperBufRd.ar(numChannels, bufnum, phase2, 0, quality);
        sig3 = SuperBufRd.ar(numChannels, bufnum, phase3, 0, quality);
        ^[XFade2.ar(XFade2.ar(sig0, sig1, pan0), XFade2.ar(sig2, sig3, pan1), pan2), phase0, playing];
    }
}

+ Buffer {
    atSec { arg secs;
        ^SuperPair.fromDouble(min(secs * sampleRate, numFrames));
    }
    atPair { arg a, b;
        ^SuperPair(a, b).asFloat / sampleRate;
    }
}
