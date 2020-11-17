SuperPair : AbstractFunction {
    var <msd, <lsd;

    *new { arg msd=0, lsd=0;
        if(this.validateInputs(msd,lsd).not){
            Error("Invalid SuperPair inputs: %, %".format(msd,lsd)).throw;
            ^nil
        };
        // pass ugens: they can only be 32 bit floats
        if(msd.isUGen and: lsd.isUGen){
            ^super.newCopyArgs(msd, lsd);
        };
        if (msd.isUGen or: lsd.isUGen) {
            if([msd,lsd].detect(_.isUGen.not) != 0){
                ^(msd + lsd).asPair
            }{
                ^super.newCopyArgs([msd,lsd].detect(_.isUGen), 0)
            }

        } {
            var double = (msd + lsd).asFloat;
            msd = Float.from32Bits(double.as32Bits);
            lsd = Float.from32Bits((double - msd).as32Bits);
            ^super.newCopyArgs(msd, lsd);
        }
    }

    *validateInputs { arg ...args;
        ^args.any{|x| x.isNil or: x.isCollection }.not
    }

    asFloat {
        if (this.isUGen) {
            "SuperPair UGen .asFloat loses double precision".warn;
        }
        ^(msd + lsd);
    }

    as32BitFloat {
        if (this.isUGen) {
            ^(msd + lsd);
        } {
            ^Float.from32Bits((msd + lsd).as32Bits);
        }
    }

    asPair { ^this }

    asBig { ^this }

    components {
        ^[msd, lsd];
    }

    asPairComponents { ^this.components }

    asOSCArgEmbeddedArray { arg array;
		    ^this.components.asOSCArgEmbeddedArray(array)
    }

    // or in other words: don't pass SuperPair around normal UGens
    // - prevent unwanted conversions/multichannel-expansion
    // - make sure SuperPair is handled only by "prepared" UGens
    // - allow performBinaryOpOnUGen to fire when doing (UGen * SuperPair)
    isValidUGenInput { ^false }

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
            SuperPair(UnaryOpFunction.new(aSelector, this.asFloat).value)
        }
    }

    composeBinaryOp { arg aSelector, something, adverb;
        ^if(this.isUGen || something.isUGen){
            SuperBinaryOpUGen(aSelector,this,something)
        }{
            SuperPair(
              BinaryOpFunction.new(aSelector, this.asFloat, something, adverb).value
            )
        }
    }
    reverseComposeBinaryOp { arg aSelector, something, adverb;
        ^if(this.isUGen || something.isUGen){
            SuperBinaryOpUGen(aSelector,something,this)
        }{
            SuperPair(
              BinaryOpFunction.new(aSelector, something, this.asFloat, adverb).value
            )
        }
    }
    // ops like (UGen * SuperPair) should return a SuperBinaryOpUGen
    performBinaryOpOnUGen { arg aSelector, aUGen;
        ^aUGen.asPair.perform(aSelector, this)
    }

    composeNAryOp { arg aSelector, anArgList;
        ^if(this.isUGen){
            ^thisMethod.notYetImplemented
        }{
            SuperPair(
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

    asString {
        ^if(this.isUGen){
            "SuperPair(%, %)".format(this.msd, this.lsd)
        }{
            "SuperPair(%)".format(this.asFloat)
        }
    }
    debug { arg caller;
		if(caller.notNil,{
			Post << caller << ": " << this.asString << this.components << Char.nl;
		},{
			Post << this.asString << this.components << Char.nl;
		});
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
        var pairArgs = a.asArray.collect(_.asPair) ++ b.asArray.collect(_.asPair);
        ^this.multiNewList(['audio', selector] ++ pairArgs);
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

    *new1 { arg rate, selector, pairA, pairB;
        var msdA,msdB,lsdA,lsdB;
        # msdA,lsdA = pairA.components;
        # msdB,lsdB = pairB.components;

        // eliminate degenerate cases
        if (selector == '*') {
            if ((msdA == 0.0) && (lsdA == 0.0), { ^0.asPair });
            if ((msdB == 0.0) && (lsdB == 0.0), { ^0.asPair });
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
