# super-bufrd
UGens for accessing long buffers with subsample accuracy:
- **SuperBufRd** A modification of BufRd to be able to access samples with double precision
- **SuperPhasor** A new phasor UGen to drive a SuperBufRd
- **SuperPhasorX** A new phasor UGen to drive multiple SuperBufRds with crossfading to allow for click-free looping and position jumping
- **SuperIndex** A sclang class to communicate position information with these UGens
- **SuperPlayBuf** A pseudo-ugen wrapper around a SuperPhasor and SuperBufRd, similar to PlayBuf
- **SuperPlayBufDetails** Same thing but outputs the phasor information as well as audio signal
- **SuperPlayBufX** / **SuperPlayBufXDetails** Like SuperPlayBuf/SuperPlayBufDetails but offers crossfading at loop points and on jumping to a new position

[Full spec / documentation of classes here](https://gist.github.com/esluyter/53597bed464d16fdb603c9db8405e3a9)

This implementation is a work in progress.

## Simple example usage of SuperPlayBufX pseudo-ugen
```
// sound file up to 2139095040 samples long (i.e. up to 12 hours long at 48k)
~buf = Buffer.read(s, "path/to/long/soundfile.wav", action: { "OK, loaded!".postln });
// wait for buffer to load then play it on a loop, mouse controls speed from 5x forward to 5x in reverse
x = { |t_trig=0, pos=#[0,0]| SuperPlayBufX.ar(2, ~buf, MouseX.kr(-5, 5), cuePos:pos, cueTrig:t_trig) }.play;
// jump to a certain position in seconds:
x.set(\pos, ~buf.atSec(230.704), \t_trig, 1);
```

## More elaborate example with playhead
```
~buf = Buffer.read(s, "path/to/long/soundfile.wav", action: { "OK, loaded!".postln });

(
~synth = {
  var cuePos = \cuePos.kr([0, 0]);
  var cueTrig = \cueTrig.tr(0);
  var rate = MouseX.kr(-12, 12);
  var lpf_freq = rate.abs.linlin(1, 3, 20000, 5000); // make fast forward less grating on ears
  var sig, playhead, isPlaying;
  #sig, playhead, isPlaying = SuperPlayBufXDetails.ar(2, ~buf, rate, cuePos: cuePos, cueTrig: cueTrig);
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

## Mac / Linux build instructions
- On a Mac, make sure you have Xcode command-line tools installed
- Make sure you have a copy of the SuperCollider source code

Run the following commands in a terminal:
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

## Windows build instructions
I'm not a Windows expert, but these are the steps that worked for me on a 64-bit version of Windows 10. If you have a 32-bit version, omit the "Win64" from the first cmake line below.

- Install git
- Install cmake
- Install visual studio community 2017 / Desktop development with C++
- Make sure you have a copy of the SuperCollider source code

Run the following commands in git bash:
```
git clone https://github.com/esluyter/super-bufrd
cd super-bufrd
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -DSC_PATH=/path/to/sc3source ..
cmake --build . --config Release
```
Move the super-bufrd folder into your `Platform.userExtensionDir` and recompile sclang.

## TODO
- Enforce size limit of 2139095040 samples
- Probably much more....
