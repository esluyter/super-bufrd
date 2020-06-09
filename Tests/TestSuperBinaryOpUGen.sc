TestSuperBinaryOpUGen : UnitTest {

  var server;

	setUp {
		server = Server(this.class.name);
    // server.bootSync; // boot only in tests that need it
	}

	tearDown {
		if(server.serverRunning) { server.quit };
		server.remove;
	}

  test_SBOUG_Order_SuperPairTimesUGen {
    var op = (3600.002.asPair*DC.kr);
		this.assertEquals(op.class, SuperPair, "SuperPair times UGen should return a SuperPair");
	}

  test_SBOUG_Order_UGenTimesSuperPair {
    var op = (DC.kr*3600.002.asPair);
		this.assertEquals(op.class, SuperPair, "UGen times SuperPair should return a SuperPair");
	}


}
