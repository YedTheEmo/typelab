# Granular synthesis - concepts

Legacy analog tape mechanisms inextricably link pitch and duration. If you halve
the playback speed of a tape, the duration doubles and the pitch drops by exactly
one octave. The PlayBuf generator inherits this assumption. Granular synthesis
was engineered specifically to decouple these two dimensions.

Granular synthesis does not play a file continuously. It mathematically slices the
buffer data into micro-acoustic fragments called grains. A typical grain lasts
between ten and one hundred milliseconds. By spawning dozens of overlapping grains
simultaneously, you synthesize a continuous texture out of discrete audio particles.

The GrainBuf unit generator executes this operation. It requires a continuous
stream of triggers to spawn grains. The Impulse generator outputs single-sample
clicks at a defined frequency. Routing an Impulse into a GrainBuf determines
the exact grain spawning rate.

    trig = Impulse.ar(20);

If the spawning rate is 20 hertz, the server generates 20 grains per second.
The independence of granular processing is achieved by manipulating the parameters
applied to each triggered grain.

When a trigger fires, GrainBuf reads the playback duration, the playback rate
(pitch), and the buffer position pointer. Crucially, these parameters are evaluated
independently per grain. You can traverse the buffer slowly by advancing the
position pointer, while simultaneously forcing each spawned grain to read its
fragment at double speed. The pitch shifts up an octave, but the overall temporal
duration of the sound remains strictly linked to the slow pointer traversal.

    GrainBuf.ar(2, trig, dur, bufnum, rate, pos);

To prevent sharp discontinuities at the boundaries of these audio fragments,
GrainBuf applies a mathematical envelope to every single grain, forcing its
amplitude to fade in and out. This windowing prevents clicking, producing
smooth, overlapping textures.

By applying randomized modulation to the position or rate arguments at the
trigger rate, you can completely dissolve a linear audio file into an abstract
cloud of spectral data. Assuming granular processing is just an effect is a
miscalculation; it is a fundamental architectural deconstruction of recorded sound.

## Next step

Now type the code version of this lesson.
