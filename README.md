# super-bufrd
UGens for accessing long buffers with subsample accuracy:
- **SuperBufRd** A modification of BufRd to be able to access samples with double precision
- **SuperPhasor** A new phasor UGen to drive a SuperBufRd
- **SuperPhasorX** A new phasor UGen to drive multiple SuperBufRds with crossfading to allow for click-free looping and position jumping
- **SuperPair** A sclang class to communicate position information with these UGens
- **SuperPlayBuf** A pseudo-ugen wrapper around a SuperPhasor and SuperBufRd, similar to PlayBuf. Optionally outputs the phasor information as well as audio signal
- **SuperPlayBufX** Like SuperPlayBuf but offers crossfading at loop points and on jumping to a new position
- **SuperBufFrames** A modification of BufFrames

This implementation is a work in progress. If you find bugs or have feature requests please submit an issue!

## Simple example usage of SuperPlayBufX pseudo-ugen
```supercollider
// sound file up to 2139095040 samples long (i.e. up to 12 hours long at 48k)
~buf = Buffer.read(s, "path/to/long/soundfile.wav", action: { "OK, loaded!".postln });

// wait for buffer to load then play it on a loop, mouse controls speed from 5x forward to 5x in reverse
x = {
  SuperPlayBufX.ar(1, ~buf, MouseX.kr(-5, 5), \trig.tr(0), start:\pos.skr(0).poll)
}.play;

// jump to a certain position in seconds:
x.set(\pos, 230.704.asPair, \trig, 1);
```

## More elaborate example with playhead
```supercollider
~buf = Buffer.read(s, "path/to/long/soundfile.wav", action: { "OK, loaded!".postln });

(
~synth = {
    var pos = \pos.skr(0);
    var trig = \trig.tr(0);
    var rate = MouseX.kr(-12, 12);
    var lpf_freq = rate.abs.linlin(1, 3, 20000, 5000); // make fast forward less grating on ears
    var sig, playhead, isPlaying;
    #sig, playhead, isPlaying = SuperPlayBufX.arDetails(2, ~buf, rate, trig, start: pos);
    SendReply.ar(Impulse.ar(10), '/playhead', playhead.components); // send both components of playhead
    LPF.ar(sig, lpf_freq);
}.play;

// print the elapsed time:
OSCdef(\playhead, { |msg|
    ("Playhead: %       BufDur: %".format(
        ~buf.atPair(*msg[3..4]).asTimeString,
        ~buf.duration.asTimeString)
    ).postln;
}, '/playhead');
)

~synth.set(\pos, (60 * 10).asPair, \trig, 1) // 10 minutes in
~synth.set(\pos, (60 * 40).asPair, \trig, 1) // 40 minutes in
~synth.set(\pos, (60 * 60).asPair, \trig, 1) // 1 hour in
```

## Build instructions

### Requirements

General requirements:
- git (for Windows, install git and git-bash)
- cmake
- a copy of the SuperCollider source code

macOS requirements:
- make sure you have Xcode command-line tools installed

Windows requirements:
- git-bash (installed with Git)
- Visual Studio Community Version / Desktop development with C++

### Non-Windows build steps

Run the following commands in a terminal:
```
git clone https://github.com/esluyter/super-bufrd.git
cd super-bufrd
mkdir build
cd build
cmake -DSC_PATH=/path/to/sc3source/ -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```
Move the super-bufrd folder into your `Platform.userExtensionDir` and recompile sclang.

### Windows build steps

Replace `<generator>` with the
[generator name](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) for your
compiler. For instance, for Visual Studio 2017 you would use `-G "Visual Studio 15 2017"`. Replace `x64` with `Win32`
for a 32-bit build.

Run the following commands in the git-bash terminal:
```
git clone https://github.com/esluyter/super-bufrd.git
cd super-bufrd
mkdir build
cd build
cmake -G "<generator>" -A x64 -DSC_PATH=/path/to/sc3source/ -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```
Move the super-bufrd folder into your `Platform.userExtensionDir` and recompile sclang.
