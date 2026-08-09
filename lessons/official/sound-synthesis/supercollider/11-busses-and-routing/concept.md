# Busses and routing - concepts

Assuming audio flows automatically from a generator to an effect implies a monolithic
architecture. SuperCollider rejects this assumption. The audio server calculates
data in a strict linear sequence, reading from and writing to a global array of
memory channels called busses. To pass a signal between two independent nodes,
you must route the data through an internal bus.

Hardware interfaces reserve the lowest bus indices. By default, bus 0 is the left
speaker and bus 1 is the right speaker. Writing to these busses sends audio directly
to the digital-to-analog converters. Internal routing requires utilizing higher bus
numbers that do not map to physical outputs. 

Manually tracking integer bus numbers is a structural liability. You must allocate
a Bus object on the client side. This object securely reserves an index on the
server and prevents other processes from overwriting your routing channels.

    b = Bus.audio(s, 2);

This command reserves a two-channel audio-rate bus. To push audio into this bus,
the source synthesis definition must dynamically assign its output target via an
argument rather than hardcoding a zero into the Out generator.

    Out.ar(outBus, sig);

To receive the audio, the effect synthesis definition must utilize the In generator,
reading from the identical bus index.

    sig = In.ar(inBus, 2);

Establishing this routing exposes a critical temporal dependency: order of
execution. The server processes the node tree linearly from top to bottom. If the
reverb effect calculates before the source oscillator, it reads empty memory. The
source then calculates and writes to the bus, but the cycle ends before the effect
can process it. This results in complete silence.

You cannot rely on the chronological order of instantiation to guarantee the correct
calculation order. You must explicitly command the server where to place each
node in the processing tree. We use the addAction parameter during instantiation
to force effects to the tail of the tree, ensuring they calculate only after all
source nodes have populated the internal busses.

    Synth(\reverb, [\inBus, b], addAction: \addToTail);

## Next step

Now type the code version of this lesson.
