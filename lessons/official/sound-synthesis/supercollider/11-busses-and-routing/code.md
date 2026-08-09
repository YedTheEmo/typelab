# Busses and routing - typing

This lesson types internal routing: allocate a bus, read it with In.ar, write
to it with Out.ar, and enforce processing order.

## Boot the server

Routing synchronizes client allocation with server execution.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for internal routing.");
    });

## Bus allocation

A stereo audio bus avoids colliding with the hardware outputs.

    s.waitForBoot({
        // allocate a two-channel audio bus on the server
        b = Bus.audio(s, 2);
    });

## Effect and source definitions

The effect reads a bus; the source writes to a chosen bus.

    s.waitForBoot({
        // compile the reverb effect
        SynthDef(\reverb, { arg inBus, outBus = 0, mix = 0.33;
            // read two channels from the input bus
            var sig = In.ar(inBus, 2);
            // apply the reverb effect
            sig = FreeVerb.ar(sig, mix, 0.9, 0.1);
            // write the wet signal to the output bus
            Out.ar(outBus, sig);
        // compile the graph and send it to the server
        }).add;

        // compile the pluck source
        SynthDef(\pluck, { arg outBus, freq = 440, amp = 0.5;
            // locals for the signal and the envelope
            var sig, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 0.5), doneAction: 2);
            // a sine shaped by the envelope
            sig = SinOsc.ar(freq) * env * amp;
            // write to the bus chosen at spawn time
            Out.ar(outBus, sig ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Order of execution

The reverb runs at the tail so sources can feed it.

    // place the reverb at the tail, reading the allocated bus
    y = Synth(\reverb, [\inBus, b.index], addAction: \addToTail);

    // spawn sources at the head, writing to the same bus
    Synth(\pluck, [\outBus, b.index, \freq, 440]);
    Synth(\pluck, [\outBus, b.index, \freq, 880]);

## Cleanup

Free the perpetual reverb and deallocate the bus.

    // free the reverb node
    y.free;
    // release the bus memory
    b.free;
    // shut down the server process
    s.quit;

## Now type it again

Type the instantiation syntax that enforces execution order.

    // place the reverb at the tail, reading the allocated bus
    y = Synth(\reverb, [\inBus, b.index], addAction: \addToTail);
    // spawn a source at the head, writing to the same bus
    Synth(\pluck, [\outBus, b.index, \freq, 330]);

## Wrap up

Bus.audio -> In.ar -> Out.ar -> addAction: \addToTail -> index routing
