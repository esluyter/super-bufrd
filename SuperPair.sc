SuperPair : AbstractFunction {
    var msd, lsd;

    *new { arg msd=0, lsd=0;
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

    asPair { ^this }

    asBig { ^this }

    components {
        ^[msd, lsd];
    }

    asOSCArgEmbeddedArray { arg array;
		    ^this.components.asOSCArgEmbeddedArray(array)
    }

    isUGen {
        ^(msd.isUGen or: lsd.isUGen)
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


    composeUnaryOp { arg aSelector;
        ^if(this.isUGen){
            thisMethod.notYetImplemented
        }{
            SuperPair.fromDouble(UnaryOpFunction.new(aSelector, this.asFloat).value)
        }
    }

    composeBinaryOp { arg aSelector, something, adverb;
        ^if(this.isUGen || something.isUGen){
            SuperBinaryOpUGen(aSelector,this,something)
        }{
            SuperPair.fromDouble(
              BinaryOpFunction.new(aSelector, this.asFloat, something, adverb).value
            )
        }
    }
    reverseComposeBinaryOp { arg aSelector, something, adverb;
        ^if(this.isUGen || something.isUGen){
            SuperBinaryOpUGen(aSelector,something,this)
        }{
            SuperPair.fromDouble(
              BinaryOpFunction.new(aSelector, something, this.asFloat, adverb).value
            )
        }
    }

    composeNAryOp { arg aSelector, anArgList;
        ^if(this.isUGen){
            ^thisMethod.notYetImplemented
        }{
            SuperPair.fromDouble(
              NAryOpFunction.new(aSelector, this.asFloat, anArgList).value
            )
        }
    }

    if { arg trueUGen, falseUGen;
        ^if(this.isUGen){
            (this * (trueUGen - falseUGen)) + falseUGen;
        }{
            Error("Non Boolean in test").throw
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
        var valueArr = value.asPair.components;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////

SuperBinaryOpUGen : BasicOpUGen {

  var <channels;

    *new { arg selector, a, b;
        ^this.multiNewList(['audio', selector] ++ a.asPair.asArray ++ b.asPair.asArray);
    }

  determineRate { arg msdA, lsdA, msdB, lsdB;
    if (msdA.rate == \demand, { ^\demand });
    if (lsdA.rate == \demand, { ^\demand });
    if (msdB.rate == \demand, { ^\demand });
    if (lsdB.rate == \demand, { ^\demand });
    if (msdA.rate == \audio, { ^\audio });
    if (lsdA.rate == \audio, { ^\audio });
    if (msdB.rate == \audio, { ^\audio });
    if (lsdB.rate == \audio, { ^\audio });
    if (msdA.rate == \control, { ^\control });
    if (lsdA.rate == \control, { ^\control });
    if (msdB.rate == \control, { ^\control });
    if (lsdB.rate == \control, { ^\control });
    ^\scalar
  }

  *new1 { arg rate, selector, msdA, lsdA, msdB, lsdB;

    // eliminate degenerate cases
    if (selector == '*') {
      if ((msdA == 0.0) && (lsdA == 0.0), { ^0.0 });
      if ((msdB == 0.0) && (lsdB == 0.0), { ^0.0 });
      if ((msdA+lsdA) == 1.0 ) { ^SuperPair(msdB,lsdB) };
      if ((msdA+lsdA) == -1.0 ) { ^SuperPair(msdB,lsdB).neg };
      if ((msdB+lsdB) == 1.0 ) { ^SuperPair(msdA,lsdA) };
      if ((msdB+lsdB) == -1.0 ) { ^SuperPair(msdA,lsdA).neg };
    }{
    if (selector == '+') {
      if ((msdA == 0.0) && (lsdA == 0.0)) { ^SuperPair(msdB,lsdB) };
      if ((msdB == 0.0) && (lsdB == 0.0)) { ^SuperPair(msdA,lsdA) };
    }{
    if (selector == '-') {
      if ((msdA == 0.0) && (lsdA == 0.0)) { ^SuperPair(msdB,lsdB).neg };
      if ((msdB == 0.0) && (lsdB == 0.0)) { ^SuperPair(msdA,lsdA) };
    }{
    if (selector == '/') {
      if ((msdB+lsdB) == 1.0 ) { ^SuperPair(msdA,lsdA) };
      if ((msdB+lsdB) == -1.0 ) { ^SuperPair(msdA,lsdA).neg };
    }}}};


    ^super.new1(rate, selector, msdA, lsdA, msdB, lsdB)
  }

  init { arg theOperator, msdA, lsdA, msdB, lsdB;
    this.operator = theOperator;
    rate = this.determineRate( msdA, lsdA, msdB, lsdB );
    inputs = [ msdA, lsdA, msdB, lsdB ];
    channels = {|i| OutputProxy(rate, this, i)}!2;
    inputs.postln;
    rate.postln;
    ^SuperPair(*channels)
  }

  numOutputs { ^2 }
  writeOutputSpecs { arg file;
    channels.do({ arg output; output.writeOutputSpec(file); });
  }
  synthIndex_ { arg index;
    synthIndex = index;
    channels.do({ arg output; output.synthIndex_(index); });
  }

}
