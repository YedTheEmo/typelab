# Synthesis definitions - concepts

Relying on the function play method is a structural liability. It creates a
temporary, anonymous graph, compiles it, and immediately instantiates it. This
approach masks the underlying architecture and prevents object reuse. To construct
reliable audio applications, you must explicitly compile Synthesis Definitions.

A SynthDef is a static, immutable graph of Unit Generators. It translates your
client-side logic into an optimized binary format the audio engine understands.
Once compiled, the topological structure of this graph is locked. You cannot
dynamically add or remove oscillators from an active node. Assuming you can
modify the signal flow after compilation reveals a fundamental misunderstanding
of the server architecture.

You construct a SynthDef by providing a global symbolic name and a graph function.

    SynthDef(\basicSine, {
        // Audio graph defined here
    });

Inside the function, you must explicitly define how audio exits the graph. The
temporary play method handles this silently, but a formal SynthDef demands an
explicit output routing command using the Out unit generator. Without this
assignment, the server calculates the audio but writes the data to null memory.

    SynthDef(\basicSine, {
        var sig = SinOsc.ar(440, 0, 0.1);
        Out.ar(0, sig);
    });

To manipulate an active node, you must expose parameters using arguments. A
standard variable is resolved and fixed at compile time. An argument creates a
control input on the compiled node, allowing it to accept external state changes
while running.

    SynthDef(\basicSine, { arg freq = 440, amp = 0.1;
        var sig = SinOsc.ar(freq, 0, amp);
        Out.ar(0, sig);
    });

Compiling the definition does not produce sound. It only generates the binary
data. You must explicitly send this data to the server using the add method.

    SynthDef(\basicSine, { ... }).add;

Once the server stores the definition in its memory registry, you instantiate
the audio process using the Synth object. You invoke it by passing the registered
name and an optional array of initial parameter values.

    Synth(\basicSine, [\freq, 330, \amp, 0.2]);

This rigorous separation of definition and instantiation allows you to spawn
thousands of identical voices from a single binary blueprint.

## Next step

Now type the code version of this lesson.
