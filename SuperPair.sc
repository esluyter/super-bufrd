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
            "SuperPair UGen .asFloat loses double precision".warn;
        }
        ^(msd + lsd);
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

    rate {
        ^msd.rate;
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
    *new { arg trig, in, label, trigid = -1;
        // TODO: fix multichannel expansion rates -- requires fixing SuperPair:asArray
        //var rate = in.asArray.collect(_.rate).unbubble;
        //this.multiNewList([rate, trig, in, label, trigid]);
        var rate = in.rate;
        this.multiNewList([rate, trig, in, label, trigid]);
        ^in;
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

////////////////////////////////////////////////////////////////////

+ Symbol {
    skr { | val, lag, fixedLag = false, spec |
      var expandedPairs = val.asCollection.collect{|v|v.asPair.asArray}.flat;
      ^NamedControl.kr(this, expandedPairs, lag, fixedLag,spec)
      .clump(2).collect(SuperPair(*_)).unbubble
    }
    sar { | val, lag, fixedLag = false, spec |
      var expandedPairs = val.asCollection.collect{|v|v.asPair.asArray}.flat;
      ^NamedControl.ar(this, expandedPairs, lag, fixedLag,spec)
      .clump(2).collect(SuperPair(*_)).unbubble
    }
}

+ Array {
    poll { |trig = 10, label, trigid = -1|
        if (label.isNil) {
            label = this.size.collect{ |index| "UGen Array [%]".format(index).asSymbol }
        };
        trig = trig.asArray;
        label = label.asArray;
        trigid = trigid.asArray;
        ^this.collect { |item, i|
            item.poll(trig.wrapAt(i), label.wrapAt(i), trigid.wrapAt(i));
        };
    }
}
