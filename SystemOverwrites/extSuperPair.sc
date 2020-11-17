+ SimpleNumber {

    asPair {
        var double = this.asFloat;
        ^SuperPair(double);
    }

    // useful for partial application
    asPairComponents {
        ^this.asPair.components;
    }

    asBig {
        ^this.asPair;
    }
}

+ UGen {
    asPair {
        ^SuperPair(this, 0)
    }

    // useful for partial application
    asPairComponents {
        ^this.asPair.components;
    }

    as32BitFloat { ^this }

    superPoll { arg trig = 10, label, trigid = -1;
        ^SuperPoll.ar(trig, this, label, trigid);
    }

    composeBinaryOp { arg aSelector, anInput;
    		if (anInput.isValidUGenInput, {
    			^BinaryOpUGen.new(aSelector, this, anInput)
    		},{
          // adding a return statement here (check out UGen:composeBinaryOp)
          // so we can return a SuperBinaryOpUGen
    			^anInput.performBinaryOpOnUGen(aSelector, this);
    		});
	  }
}

+ Symbol {
    skr { | val, lag, fixedLag = false, spec |
        var expandedPairs = val.asArray.collect(_.asPairComponents).flat;
        ^NamedControl.kr(this, expandedPairs, lag, fixedLag,spec)
        .clump(2).collect(SuperPair(*_)).unbubble
    }
    sar { | val, lag, fixedLag = false, spec |
        var expandedPairs = val.asArray.collect(_.asPairComponents).flat;
        ^NamedControl.ar(this, expandedPairs, lag, fixedLag,spec)
        .clump(2).collect(SuperPair(*_)).unbubble
    }
}

+ Array {

    as32BitFloat { ^ this collect: _.as32BitFloat }

    superPoll { arg trig = 10, label, trigid = -1;
        if (label.isNil) { label = this.collect{ |thing, index| "Array [%] (%)".format(index, thing.class) }};
        ^SuperPoll.ar(trig, this, label, trigid);
    }

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

+ Buffer {
    atSec { arg secs;
        ^SuperPair(min(secs * sampleRate, numFrames));
    }
    atPair { arg a, b;
        ^SuperPair(a, b).asFloat / sampleRate;
    }
}
