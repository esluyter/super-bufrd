// without mul and add.
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
