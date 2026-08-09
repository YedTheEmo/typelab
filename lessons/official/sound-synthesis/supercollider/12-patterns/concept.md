# Patterns - concepts

Manually spawning individual synth nodes via the client interface is a failure of
automation. Assuming human timing can achieve microsecond precision for musical
sequencing is a flaw. To construct complex temporal structures, you must abandon
direct node instantiation and utilize the Pattern library.

The core of algorithmic sequencing is the Pbind object. A Pbind does not generate
audio. It generates streams of data, sequentially compiling parameter arrays and
transmitting them to the server to spawn nodes exactly on schedule.

    Pbind(
        \instrument, \mySynth,
        \freq, 440,
        \dur, 0.5
    );

A Pbind evaluates key-value pairs. The instrument key explicitly links the pattern
to a compiled SynthDef residing in server memory. The dur key is structurally
mandatory; it dictates the temporal delta between each spawned node in beats.
If you omit the instrument key, SuperCollider falls back to a default piano-like
synth, which obscures routing errors. 

Static numbers produce repetitive output. The true power of Patterns lies in
nesting other Pattern objects within the Pbind to generate dynamic algorithmic
streams. The Pseq object iterates through an array of values in strict sequential
order, repeating a specified number of times.

    \freq, Pseq([220, 330, 440], 4)

The Prand object selects a value from an array entirely at random upon every
iteration, destroying predictability. 

    \amp, Prand([0.1, 0.3, 0.5], inf)

Because Pbind spawns a new Synth node for every event, the target SynthDef must
manage its own lifecycle. If the definition lacks an envelope with a doneAction
of 2, the pattern will spawn thousands of perpetual nodes, saturating the CPU
and crashing the server within seconds.

Evaluating a Pbind only defines the algorithmic structure. It does nothing until
you execute the play method, which binds the pattern to a scheduling clock and
begins transmitting the generated events across the network boundary to the server.

## Next step

Now type the code version of this lesson.
