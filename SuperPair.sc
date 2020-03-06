SuperPair {
    var msd, lsd;

    *new { arg msd, lsd;
        ^super.newCopyArgs(msd, lsd);
    }

    *fromDouble { arg double = 0.0;
        var msd = Float.from32Bits(double.as32Bits);
        var lsd = Float.from32Bits((double - msd).as32Bits);
        ^super.newCopyArgs(msd, lsd);
    }

    asFloat {
        if (this.isUGen) {
            "Cannot convert UGen to Float".postln;
        } {
            ^(msd + lsd);
        };
    }

    asPair {
        ^this;
    }

    asArray {
        ^[msd, lsd];
    }

    asBig {
        ^this;
    }

    asOSCArgEmbeddedArray { | array|
		    array = array.add($[);
		    this.asArray.do{ | e | array = e.asOSCArgEmbeddedArray(array) };
		    ^array.add($])
    }

    isUGen {
        if (msd.isUGen or: lsd.isUGen) {
            ^true;
        };
        ^false;
    }

    poll { arg trig = 10, label, trigid = -1;
        ^SuperPoll.ar(trig, this, label, trigid);
    }

    superPoll { arg trig = 10, label, trigid = -1;
        ^this.poll(trig, label, trigid);
    }

    + { arg something;
        if (this.isUGen or: something.isUGen) {
            "UGen SuperPair addition not yet implemented".postln;
            ^this;
        } {
            ^SuperPair.fromDouble(this.asFloat + something.asFloat);
        }
    }
}

////////////////////////////////////////////////////////////////////

SuperPoll : UGen {
    *ar { arg trig, value, label, trigid = -1;
        this.multiNew('audio', trig, value, label, trigid);
        ^value;
    }
    *kr { arg trig, value, label, trigid = -1;
        this.multiNew('control', trig, value, label, trigid);
        ^value;
    }
    *new1 { arg rate, trig, value, label, trigid;
        var valueArr = value.asPair.asArray;
        label = label ?? { "%".format(value.class) };
        label = label.asString.collectAs(_.ascii, Array);
        if (trig.isNumber) { trig = Impulse.multiNew(rate, trig, 0) };
        ^super.new.rate_(rate).addToSynth.init([trig] ++ valueArr ++ [trigid, label.size] ++ label);
    }
    init { arg theInputs;
        inputs = theInputs;
        //^this.initOutputs(2, rate);
    }
}
