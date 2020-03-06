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
        ^SuperPair(this, 0.0)
    }

    superPoll { arg trig = 10, label, trigid = -1;
        ^SuperPoll.ar(trig, this, label, trigid);
    }
}

+ Symbol {
    krBig { arg value = 0.0;
        ^SuperPair(*this.kr(value.asPair))
    }
}

+ Array {
    superPoll { arg trig = 10, label, trigid = -1;
        if (label.isNil) { label = this.collect{ |thing, index| "Array [%] (%)".format(index, thing.class) }};
        ^SuperPoll.ar(trig, this, label, trigid);
    }
}
