SuperPhasor : MultiOutUGen {
    *ar { arg trig=0, rate=1, startInt=0, startDec=0, endInt, endDec=0, resetInt=0, resetDec=0, loop=1;
        ^this.multiNew('audio', trig, rate, startInt, startDec, endInt, endDec, resetInt, resetDec, loop)
    }
    init { arg ... theInputs;
        inputs = theInputs;
        ^this.initOutputs(3, rate);
    }
}

SuperBufRd : MultiOutUGen {
    *ar { arg numChannels=1, bufnum=0, phaseInt=0, phaseDec=0, loop=1, interpolation=2;
        ^this.multiNew('audio', numChannels, bufnum, phaseInt, phaseDec, loop, interpolation)
    }

    init { arg argNumChannels ... theInputs;
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

SuperBufFrames : BufInfoUGenBase {}


SuperPlayBuf {
    *ar { arg numChannels=1, bufnum=0, rate=1, startPos=[0,0], endPos=nil, cuePos=[0,0], cueTrig=0, loop=0, interpolation=0;
        var phaseInt, phaseDec, playing;
        startPos = startPos.asArray;
        endPos = endPos ? [SuperBufFrames.kr(bufnum), 0];
        endPos = endPos.asArray;
        cuePos = cuePos.asArray;
        rate = BufRateScale.kr(bufnum) * rate;
        # phaseInt, phaseDec, playing = SuperPhasor.ar(
            cueTrig, rate,
            startPos[0], startPos[1],
            endPos[0], endPos[1],
            cuePos[0], cuePos[1],
            loop);
        ^SuperBufRd.ar(numChannels, bufnum, phaseInt, phaseDec, 0, interpolation);
    }
}

SuperPlayBufDetails {
    *ar { arg numChannels=1, bufnum=0, rate=1, startPos=[0,0], endPos=nil, cuePos=[0,0], cueTrig=0, loop=0, interpolation=0;
        var phaseInt, phaseDec, playing;
        startPos = startPos.asArray;
        endPos = endPos ? [SuperBufFrames.kr(bufnum), 0];
        endPos = endPos.asArray;
        cuePos = cuePos.asArray;
        rate = BufRateScale.kr(bufnum) * rate;
        # phaseInt, phaseDec, playing = SuperPhasor.ar(
            cueTrig, rate,
            startPos[0], startPos[1],
            endPos[0], endPos[1],
            cuePos[0], cuePos[1],
            loop);
        ^[SuperBufRd.ar(numChannels, bufnum, phaseInt, phaseDec, 0, interpolation), [phaseInt, phaseDec], playing];
    }
}

SuperIndex {
    var sampleNum, sampleRate;

    *new { arg sampleNum=0, sampleRate;
        sampleRate = sampleRate ? Server.default.sampleRate;
        ^super.newCopyArgs(sampleNum, sampleRate);
    }

    *fromSecs { arg secs=0, sampleRate;
        sampleRate = sampleRate ? Server.default.sampleRate;
        ^super.newCopyArgs(secs * sampleRate, sampleRate);
    }

    *fromPair { arg pair, sampleRate;
        sampleRate = sampleRate ? Server.default.sampleRate;
        ^super.newCopyArgs(pair[0].as32Bits + pair[1], sampleRate);
    }

    asSecs { ^(sampleNum / sampleRate) }

    asDouble { ^sampleNum }

    asPair {
        var numIntAsFloat = Float.from32Bits(floor(sampleNum));
        var numDec = sampleNum - floor(sampleNum);
        ^[numIntAsFloat, numDec];
    }

    asArray { ^this.asPair }

    asUGenInput { ^this.asPair }
    asOSCArgEmbeddedArray { | array|
		array = array.add($[);
		this.asPair.do{ | e | array = e.asOSCArgEmbeddedArray(array) };
		^array.add($])
	}

    printOn { arg stream;
		stream << this.class.name << "(" <<* [sampleNum, sampleRate] <<")"
	}

}

+ Buffer {
    atSec { arg secs;
        ^SuperIndex(min(secs * sampleRate, numFrames), sampleRate);
    }
    atPair { arg pair;
        ^SuperIndex.fromPair(pair, sampleRate);
    }
}
