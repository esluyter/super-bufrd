SuperPhasor : MultiOutUGen {
    *ar { arg trig=0, rate=1, start=0, end=48000, reset=0, loop=1;
        ^this.multiNew('audio', trig, rate, start, end, reset, loop, false);
    }
    *arDetails { arg trig=0, rate=1, start=0, end=48000, reset=0, loop=1;
        ^this.multiNew('audio', trig, rate, start, end, reset, loop, true);
    }
    *new1 { arg ugen_rate, trig, rate, start, end, reset, loop, details;
        var msd, lsd, isPlaying;
        start = start.asPair.components;
        end = end.asPair.components;
        reset = reset.asPair.components;
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
        start = start.asPair.components;
        end = end.asPair.components;
        reset = reset.asPair.components;
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
        phase = phase.asPair.components;
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


SuperBufWr : UGen {
    *ar { arg inputArray, bufnum=0, phase=0, loop=1;
        phase = phase.asPair.components.postln;
        ^this.multiNewList((['audio', bufnum] ++ phase ++ [loop] ++ inputArray.asArray).postln)
    }
    *kr { arg inputArray, bufnum=0, phase=0, loop=1;
        phase = phase.asPair.components;
        ^this.multiNewList(['control', bufnum] ++ phase ++ [loop]  ++ inputArray.asArray)
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


SuperPlayBuf : MultiOutUGen{
    *ar { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2;
        var pos;
        end = end ? SuperBufFrames.kr(bufnum);
        rate = BufRateScale.kr(bufnum) * rate;
        pos = SuperPhasor.ar(trig, rate, start, end, reset, loop);
        ^SuperBufRd.ar(numChannels, bufnum, pos, 0, quality);
    }

    *arDetails{arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2;
      ^this.multiNew(
			   numChannels,bufnum,rate,trig,reset,start,end,loop,quality
      )
    }

    // only called by arDetails
    *new1 { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2;
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


// copied from wslib, then adapted
// fades (single) on loop points +
// crossfades (dual) on trigger

SuperPlayBufCF : MultiOutUGen{

    classvar <fadeFuncs;
    *initClass {
        fadeFuncs = Dictionary[
            \linear-> #{|x|x},
            \equal -> #{|x|sin(x*pi/2)},
            \sqrt  -> #{|x|sqrt(x)}
        ]
    }
	// dual play buf which crosses from 1 to the other at trigger
    *ar { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2,fadeTime=0.1, fadeFunc=\sqrt, n= 2;
		^this.multiNew(
			numChannels,bufnum,rate,trig,
			reset,start,end,loop,quality,fadeTime,fadeFunc,n
		)
	}

	*arDetails{arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2,fadeTime=0.1, fadeFunc=\sqrt, n = 2;
		^this.multiNew(
			numChannels,bufnum,rate,trig,
			reset,start,end,loop,quality,fadeTime,fadeFunc,n,true
		)
	}

	*new1 { arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2,fadeTime=0.1, fadeFunc=\sqrt, n = 2, details=false;

		var index, method = \ar, on, fadeTimeR;

		switch ( trig.rate,
			\audio, {
                index = Stepper.ar( trig, 0, 0, n-1 );
			},
			\control, {
				index = Stepper.kr( trig, 0, 0, n-1 );
				method = \kr;
			},
			\demand, {
				trig = TDuty.ar( trig ); // audio rate precision for demand ugens
				index = Stepper.ar( trig, 0, 0, n-1 );
			},
			{ ^this.prMakeFadingPlayBuf( numChannels, bufnum, rate, trig, reset,start, end, loop, quality, fadeTime, details ) } // bypass
		);

        if(fadeFunc.isSymbol){
            fadeFunc = this.fadeFuncs[fadeFunc];
        };
        if(fadeFunc.isFunction.not){
            fadeFunc = this.fadeFuncs[\sqrt];
        };

		on = n.collect({ |i|
			//on = (index >= i) * (index <= i); // more optimized way?
			InRange.perform( method, index, i-0.5, i+0.5 );
		});

		switch ( rate.rate,
			\demand,  {
				rate = on.collect({ |on, i|
					Demand.perform( method, on, 0, rate );
				});
			},
			\control, {
				rate = on.collect({ |on, i|
					Gate.kr( rate, on ); // hold rate at crossfade
				});
			},
			\audio, {
				rate = on.collect({ |on, i|
					Gate.ar( rate, on );
				});
			},
			{
				rate = rate.asCollection;
			}
		);

		if( start.rate == \demand ) {
			start = Demand.perform( method, trig, 0, start )
		};

		fadeTimeR = fadeTime.reciprocal;
		if(details){
			var players, pos;
			#players, pos =
				this.prMakeFadingPlayBuf(
					numChannels, bufnum, rate, on,
					reset,start, end, loop, quality, fadeTime, true
				);
			players = Mix(
                players * fadeFunc.value(Slew.perform( method, index, fadeTimeR, fadeTimeR ))
            );
			pos = Select.kr(on[1],pos);
			^[players,pos];
		}{

			^Mix(
				this.prMakeFadingPlayBuf( numChannels, bufnum, rate, on, reset,start, end, loop, quality, fadeTime )
                * fadeFunc.value(
                    Slew.perform(method, on, fadeTimeR, fadeTimeR)
                )
			);
		}

	}


	*prMakeFadingPlayBuf{arg numChannels=1, bufnum=0, rate=1, trig=0, reset=0, start=0, end=nil, loop=1, quality=2, fadeTime=0.1, details=false;
        var pos,pos_f,fade_f,loop_fade,start_f,end_f,player;
        start_f = start.asFloat;
        end = end ? SuperBufFrames.kr(bufnum);
		end_f = end.asFloat;
        rate = BufRateScale.kr(bufnum) * rate;
        pos = SuperPhasor.ar(trig, rate, start, end, reset, loop);
		pos_f = pos.asFloat;
		fade_f = fadeTime * BufSampleRate.ir(bufnum) / rate;
		loop_fade = pos_f.linlin(start_f,start_f+fade_f,0,1)*pos_f.linlin(end_f-fade_f,end_f,1,0);
		player = SuperBufRd.ar(numChannels, bufnum, pos, 0, quality)*loop_fade;
		if(details){
			^[player,pos_f]
		}{
			^player
		};

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
