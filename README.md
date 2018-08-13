# super-bufrd
**SuperPhasor.ar** and **SuperBufRd.ar**, UGens for accessing long buffers with subsample accuracy.

To get around 32-bit floating point numbers' lack of precision, each position value is represented by two numbers, an integer value which is a 32-bit int recast as a float, and a decimal value which is a float between 0 and 1. These are internally added together to make a double.

## Example usage using SuperPlayBuf pseudo-ugen
```
// sound file up to 2139095040 samples long (i.e. up to 12 hours long at 48k)
~buf = Buffer.read(s, "path/to/long/soundfile.wav", action: { "OK, loaded!".postln });

// wait for file to load....

(
~synth = {
  var cuePos = \cuePos.kr([0, 0]);
  var cueTrig = \cueTrig.tr(0);
  var rate = MouseX.kr(-12, 12);
  var lpf_freq = rate.abs.linlin(1, 3, 20000, 5000); // make fast forward less grating on ears
  var sig, playhead, isPlaying;
  #sig, playhead, isPlaying = SuperPlayBufDetails.ar(2, ~buf, rate, cuePos: cuePos, cueTrig: cueTrig);
  SendTrig.ar(Impulse.ar(10), 0, playhead[0]); // send integer component of playhead
  LPF.ar(sig, lpf_freq);
}.play;

// print the elapsed time:
OSCdef(\trig, { |msg|
  if (msg[2] == 0) {
    ("Start: %       Playhead: %       End: %".format(
      0.asTimeString,
      ~buf.atPair([msg[3], 0]).asSecs.asTimeString,
      ~buf.duration.asTimeString)
    ).postln;
  };
}, '/tr');
)

~synth.set(\cuePos, ~buf.atSec(60 * 10), \cueTrig, 1) // 10 minutes in
~synth.set(\cuePos, ~buf.atSec(60 * 40), \cueTrig, 1) // 40 minutes in
~synth.set(\cuePos, ~buf.atSec(60 * 60), \cueTrig, 1) // 1 hour in
```

## Same thing using the SuperPhasor and SuperBufRd ugens directly
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
    resetInt, 0, // time to move the playhead, start pos
    loop: 1 // stop playing at beginning and end of buffer
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
- Loop with crossfade
- Probably much more....
