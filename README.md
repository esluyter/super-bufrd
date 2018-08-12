# super-bufrd
**SuperPhasor.ar** and **SuperBufRd.ar**, UGens for accessing long buffers with subsample accuracy. 

To get around 32-bit floating point numbers' lack of precision, each position value is represented by two numbers, an integer value which is a 32-bit int recast as a float, and a decimal value which is a float between 0 and 1. These are internally added together to make a double.

On the sclang side, you can recast an integer as a float to be understood by these UGens using `Float.from32Bits(integer_value)`, and you can recast a float coming back from these UGens using `integer_value = float_value.as32Bits`.

## Example usage
```
// sound file up to 2139095040 samples long (i.e. up to 12 hours long at 48k)
~buf = Buffer.read(s, "path/to/long/soundfile.wav", action: { "OK, loaded!".postln });

// wait for file to load....

(
~synth = { |t_trig, resetInt|
  var phaseInt, phaseDec, playing;
  var rate = MouseX.kr(-12, 12);
  var lpf_freq = rate.abs.linlin(1, 3, 20000, 5000); // make fast forward less grating on ears
  # phaseInt, phaseDec, playing = SuperPhasor.ar(
    t_trig, // Trigger to move playhead
    BufRateScale.kr(~buf) * rate,
    0, 0, // start of section to play or loop (beginning of buffer)
    Float.from32Bits(~buf.numFrames), 0,  // end of section to play or loop (end of buffer)
    resetInt, 0, // time to move the playhead
    loop: 0 // stop playing at beginning and end of buffer
  );

  // send phase information 10x per second - only care about integer (sample number)
  SendTrig.ar(Impulse.ar(10), 0, phaseInt);

  LPF.ar(
    SuperBufRd.ar(2, ~buf, phaseInt, phaseDec, 0, 2),
    lpf_freq
  );
}.play;

// print the elapsed time:
OSCdef(\trig, { |msg|
  if (msg[2] == 0) {
    ("Start: %       Playhead: %       End: %".format(
      0.asTimeString,
      (msg[3].as32Bits / ~buf.sampleRate).asTimeString,
      ~buf.duration.asTimeString)
    ).postln;
  };
}, '/tr');

~setPlayhead = { |secs|
  ~synth.set(\resetInt, Float.from32Bits(~buf.sampleRate * secs), \t_trig, 1);
};
)

~setPlayhead.(60 * 20); // 20 minutes in
~setPlayhead.(60 * 40); // 40 minutes in
~setPlayhead.(60 * 60); // 1 hour in
```

## Build instructions
Make sure you have a copy of the SuperCollider source code, then run the following commands:
```
git clone https://github.com/esluyter/super-bufrd.git
cd super-bufrd
mkdir build
cd build
cmake -DSC_PATH=/path/to/sc3source/ ..
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
```
Move the super-bufrd folder into your `Platform.userExtensionDir` and recompile sclang.

## TODO
- Enforce size limit of 2139095040 samples
- Support audio-rate trig and rate inputs to SuperPhasor.ar
- Probably much more....
