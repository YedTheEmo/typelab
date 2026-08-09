# Envelopes - concepts

Leaving synthesis nodes active indefinitely is an architectural failure. A signal
multiplied by zero produces silence, but the audio engine continues calculating
that silence at the standard audio rate. In a polyphonic system, accumulating
hundreds of silent nodes will inevitably saturate the processor, resulting in
catastrophic audio dropouts. You must manage the lifecycle of a node and its
amplitude concurrently to preserve system resources. You accomplish this using
envelopes.

An envelope consists of two distinct components. The first is the blueprint,
defined by the Env object. This object strictly describes a sequential array of
levels and duration times.

    Env.perc(0.01, 1.0);

This specification describes a standard percussive shape: a ten-millisecond
attack to maximum amplitude, followed by a one-second decay back to zero. The
Env object itself generates no data stream. It remains a static specification
in memory.

The second component is the Unit Generator that interprets this blueprint.
EnvGen reads the specification and outputs a dynamic control signal over time.

    EnvGen.kr(Env.perc(0.01, 1.0));

This control rate generator scales the amplitude of an audio rate oscillator.
However, when the decay phase concludes, the output simply holds at zero while
the parent node continues executing. You must explicitly instruct the server to
deallocate the memory and terminate the node when the envelope finishes.

    EnvGen.kr(Env.perc(0.01, 1.0), doneAction: 2);

The doneAction parameter dictates the structural consequence of the envelope
completing its final phase. A value of two forces the server to free the
enclosing synth node immediately. This automated garbage collection is mandatory
for stable systems.

Sustaining envelopes introduce a temporal dependency. An ADSR envelope pauses
at its sustain level indefinitely.

    Env.adsr(0.1, 0.2, 0.5, 1.0);

To progress past the sustain phase, the EnvGen requires a trigger transition.
You map this to a synthesis argument called a gate. When the client drives the
gate value to zero, the envelope enters its release phase and subsequently
triggers the doneAction to terminate the node.

Assuming envelopes exist solely to modulate amplitude is a conceptual error.
An EnvGen outputs a pure mathematical signal. By routing this signal to the
frequency argument of an oscillator, you produce dynamic pitch sweeps. Envelope
generators dictate structural change across any parameter.

## Next step

Now type the code version of this lesson.
