# Patterns - typing

This lesson types algorithmic sequencing: a percussive SynthDef, a Pbind
pattern, and the play and stop lifecycle.

## Boot the server

Patterns run on the client but need an active server to render events.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for algorithmic sequencing.");
    });

## The target definition

The pattern spawns nodes from a percussive SynthDef.

    s.waitForBoot({
        // compile the ping definition
        SynthDef(\ping, { arg freq = 440, amp = 0.2;
            // locals for the signal and the envelope
            var sig, env;
            // one-shot envelope; doneAction 2 frees the node
            env = EnvGen.kr(Env.perc(0.01, 0.2), doneAction: 2);
            // a sine shaped by the envelope
            sig = SinOsc.ar(freq) * env * amp;
            // duplicate to both channels
            Out.ar(0, sig ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Defining the pattern

Pbind binds pattern streams to synth arguments.

    // a pattern that plays the ping instrument
    p = Pbind(
        // the instrument to trigger
        \instrument, \ping,
        // an endlessly looping four-note arpeggio
        \freq, Pseq([220, 330, 440, 550], inf),
        // one event every quarter second
        \dur, 0.25,
        // a uniform random amplitude for each event
        \amp, Pwhite(0.05, 0.2, inf)
    );

## Execution and termination

Play turns the blueprint into an active player.

    // start the player and keep its reference
    x = p.play;

    // stop the algorithmic stream
    x.stop;
    // shut down the server process
    s.quit;

## Now type it again

Type the core pattern definition; Pseq handles order, Pwhite handles range.

    // a pattern that loops two notes indefinitely
    p = Pbind(
        // the instrument to trigger
        \instrument, \ping,
        // an alternating two-note sequence
        \freq, Pseq([440, 880], inf),
        // one event every half second
        \dur, 0.5,
        // a uniform random amplitude for each event
        \amp, Pwhite(0.1, 0.3, inf)
    );

## Wrap up

SynthDef doneAction -> Pbind -> \instrument -> \dur -> Pseq / Pwhite -> play
