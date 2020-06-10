TestSuperPair : UnitTest {

    var server;

    setUp {
        server = Server(this.class.name);
    }

    tearDown {
        if(server.serverRunning) { server.quit };
        server.remove;
    }

    test_constructor {
        var testItems = (zero:0, int:999999999, double:3600.0002, pair:SuperPair(3600.002));
        testItems.keysValuesDo{|name_x,x|
            this.assertFloatEquals(
                SuperPair(x).asFloat,x.asFloat,"SuperPair(%)".format(name_x)
            );
            testItems.keysValuesDo{|name_y,y|
                this.assertFloatEquals(
                    SuperPair(x,y).asFloat,(x+y).asFloat,
                    "SuperPair(%,%)".format(name_x,name_y)
                )
            };
        }.flat
    }


    test_constructorUGen {
        var items = (zero:0, int:9999999, double:3600.0002, pair:SuperPair(3600.002));
        var ugens = (audio:{DC.ar(1)},control:{DC.kr(1)});
        var condition = Condition();
        server.bootSync;

        ugens.keysValuesDo{|rate,ugen|
            var dur = if(rate == 'control'){64}{1}/server.sampleRate;
            {SuperPair(ugen.value).components}.loadToFloatArray(dur,server){|v|
                this.assertFloatEquals(v[..1].sum, 1.0, "SuperPair(%)".format(rate));
                condition.unhang
            };
            condition.hang;
            items.keysValuesDo{|name, x|
                {SuperPair(ugen.value,x).components}.loadToFloatArray(dur,server){|v|
                    this.assertFloatEquals(v[..1].sum, 1.0+x.asFloat,
                        "SuperPair(%+%)".format(rate,name)
                    );
                    condition.unhang
                };
                condition.hang;
            }
        };
    }

    test_constructorUGenScalar {
        var items = (zero:0, int:9999999, double:3600.0002, pair:SuperPair(3600.002));
        var ugen = {Control.ir(1)};
        var condition = Condition();
        server.bootSync;
        {
            SuperPair(ugen.value).components.collect(DC.kr(_))
        }.loadToFloatArray(64/server.sampleRate,server){|v|
            this.assertFloatEquals(v[..1].sum, 1.0, "SuperPair(scalar)");
            condition.unhang
        };
        condition.hang;
        items.keysValuesDo{|name, x|
            {
                SuperPair(ugen.value,x).components.collect(DC.kr(_))

            }.loadToFloatArray(64/server.sampleRate,server){|v|
                this.assertFloatEquals(v[..1].sum, 1.0+x.asFloat,
                    "SuperPair(scalar+%)".format(name)
                );
                condition.unhang
            };
            condition.hang;
        }
    }


    test_Symbol_skrDefaultValue {
        var condition = Condition();
        var expected = 3600.00002;
        var result;
        server.bootSync;
        {\pos.skr(expected).components}.loadToFloatArray(64/server.sampleRate,server)
        {|val|
            result = val[..1].sum.postln;
            condition.unhang;
        };
        condition.hang;
        this.assertFloatEquals(result, expected, "Symbol.skr should output precise default value");
    }

    test_Symbol_skrSet {
        var condition = Condition();
        var expected = 3600.00002;
        var result, bus, synth;
        server.bootSync;
        bus = Bus.control(server,2);
        synth = {|bus|
            Out.kr(bus,\pos.skr(0).components)
        }.play(server);
        server.sync;
        synth.set(\bus,bus,\pos,expected.asPair);
        server.sync;
        synth.free;
        result = bus.getnSynchronous(2);
        result = result[..1].sum.postln;
        bus.free;
        this.assertFloatEquals(result, expected, "Symbol.skr should output precise value after set");
    }


}
