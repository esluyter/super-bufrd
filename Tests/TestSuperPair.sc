TestSuperPair : UnitTest {

  var server;

	setUp {
		server = Server(this.class.name);
    server.bootSync;
	}

	tearDown {
		if(server.serverRunning) { server.quit };
		server.remove;
	}


  test_SuperPair_skrDefaultValue {
    var condition = Condition();
    var expected = 3600.00002;
    var result;
    {\pos.skr(expected).components}.loadToFloatArray(64/server.sampleRate,server)
    {|val|
      result = val[..1].sum.postln;
      condition.unhang;
    };
    condition.hang;
		this.assertFloatEquals(result, expected, "NamedControl.skr should output precise default value");
	}

  test_SuperPair_skrSet {
    var condition = Condition();
    var expected = 3600.00002;
    var result, bus, synth;
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
		this.assertFloatEquals(result, expected, "NamedControl.skr should output precise value after set");
	}


}
