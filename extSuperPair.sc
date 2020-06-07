+ SimpleNumber {
    asPair {
        var double = this.asFloat;
        ^SuperPair.fromDouble(double);
    }

    asBig {
        ^this.asPair;
    }
}

+ UGen {
    asPair {
		    var method = switch(rate) {\audio}{\ar} {\control}{\kr};
        ^SuperPair(this, DC.perform(method.postln,0))
    }

    superPoll { arg trig = 10, label, trigid = -1;
        ^SuperPoll.ar(trig, this, label, trigid);
    }
}

+ Symbol {
    skr { | val, lag, fixedLag = false, spec |
        var expandedPairs = val.asArray.collect{|v|v.asPair.components}.flat;
        ^NamedControl.kr(this, expandedPairs, lag, fixedLag,spec)
        .clump(2).collect(SuperPair(*_)).unbubble
    }
    sar { | val, lag, fixedLag = false, spec |
        var expandedPairs = val.asArray.collect{|v|v.asPair.components}.flat;
        ^NamedControl.ar(this, expandedPairs, lag, fixedLag,spec)
        .clump(2).collect(SuperPair(*_)).unbubble
    }
}

+ Array {
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