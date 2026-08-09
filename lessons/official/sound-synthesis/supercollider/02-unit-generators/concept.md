# Unit generators - concepts

The prevailing assumption that digital sound exists as a continuous physical wave
is mathematically incorrect. Sound inside a digital system is a discrete sequence of
numerical values calculated at fixed time intervals. To synthesize audio, we
rely on Unit Generators. These are optimized C++ objects executing on the server
that produce or manipulate these numerical streams.

Every Unit Generator demands a specific calculation rate. Assuming that all
signals require maximum resolution is a waste of processing resources. We divide
operations strictly based on the necessary temporal fidelity.

The audio rate method ar instructs the server to calculate one value for every
single audio sample. At a standard sample rate, this forces 48,000 calculations
per second. This high calculation density is mandatory for signals routed directly
to the audio interface and speakers.

The control rate method kr calculates one value per control block, which defaults
to 64 samples. This reduces the processing load significantly. We apply control
rate strictly to low-frequency modulation or parameter changes where human
perception cannot detect the reduced numerical resolution.

Unit Generators take specific arguments to define their behavior. A sine oscillator
requires a frequency, a phase offset, and a multiplier to scale its amplitude.

    SinOsc.ar(440, 0, 0.1);

This statement generates a pure fundamental frequency at 440 hertz, scaled down
to ten percent of the maximum digital threshold to prevent hardware clipping.

Complex waveforms provide richer harmonic data. A sawtooth oscillator generates
all integer harmonics, producing a dense spectrum suitable for subtractive
filtering algorithms.

    Saw.ar(100, 0.1);

A pulse oscillator allows dynamic adjustment of its duty cycle, altering the
ratio of the high and low states within a single period. This alters the odd
and even harmonic content over time.

    Pulse.ar(100, 0.5, 0.1);

Every Unit Generator inherits a standard interface for signal scaling. The
final two arguments are invariably mul and add. The mul argument multiplies the
output signal, controlling amplitude. The add argument applies a direct numerical
offset, shifting the baseline signal voltage.

SuperCollider implements an architecture called multi-channel expansion. If you
supply an array of values to a Unit Generator instead of a single integer, the
engine does not throw a type error. It silently duplicates the generator entirely.

    SinOsc.ar([440, 444], 0, 0.1);

This mechanism creates two independent sine oscillators, routing the first to the
left channel and the second to the right. Assuming a single variable always maps
to a single audio stream will lead to catastrophic routing errors in complex
graphs. You must continuously interrogate the data types passed into the arguments.

## Next step

Now type the code version of this lesson.
