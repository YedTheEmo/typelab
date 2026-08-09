# Envelopes - typing

This lesson types envelope control: percussive envelopes that self-terminate,
sustained ADSR envelopes that need a gate, and envelopes driving pitch.

## Boot the server

Envelope structures require an initialized audio engine.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Server initialized.");
    });

## Percussive definitions

Env.perc makes a fixed-duration amplitude curve.

    s.waitForBoot({
        // compile a percussive tone definition
        SynthDef(\percTone, { arg freq = 440, amp = 0.5;
            // local variables for signal and envelope
            var sig, env;
            // one-shot envelope; doneAction 2 frees the node
            env = EnvGen.kr(Env.perc(0.01, 1.0), doneAction: 2);
            // shape the oscillator with the envelope
            sig = SinOsc.ar(freq) * env * amp;
            // send the signal to the hardware output
            Out.ar(0, sig);
        // compile the graph and send it to the server
        }).add;
    });

## Sustained definitions

An ADSR envelope holds at sustain while the gate is above zero.

    s.waitForBoot({
        // compile a sustained tone definition
        SynthDef(\sustainedTone, { arg freq = 220, amp = 0.5, gate = 1;
            // local variables for signal and envelope
            var sig, env;
            // ADSR driven by the gate; release then free the node
            env = EnvGen.kr(Env.adsr(0.1, 0.2, 0.5, 1.0), gate, doneAction: 2);
            // a sawtooth shaped by the envelope
            sig = Saw.ar(freq) * env * amp;
            // send the signal to the hardware output
            Out.ar(0, sig);
        // compile the graph and send it to the server
        }).add;
    });

## Pitch envelopes

An envelope can drive frequency instead of amplitude.

    s.waitForBoot({
        // compile a drum definition
        SynthDef(\kick, { arg amp = 0.8;
            // local variables for signal and envelopes
            var sig, ampEnv, freqEnv;
            // exponential pitch drop from 1000 to 50 hertz
            freqEnv = EnvGen.kr(Env([1000, 50], [0.5], \exp));
            // the amplitude envelope alone owns the doneAction
            ampEnv = EnvGen.kr(Env.perc(0.01, 1.0), doneAction: 2);
            // the frequency envelope drives the oscillator
            sig = SinOsc.ar(freqEnv) * ampEnv * amp;
            // send the signal to the hardware output
            Out.ar(0, sig);
        // compile the graph and send it to the server
        }).add;
    });

## Lifecycle execution

Self-terminating nodes need no gate or variable.

    // one-shot percussive tone at 880 hertz
    Synth(\percTone, [\freq, 880]);
    // one-shot kick drum
    Synth(\kick);

The sustained node requires explicit gate control.

    // spawn a sustained tone and keep its reference
    x = Synth(\sustainedTone, [\freq, 110]);
    // close the gate to begin the release phase
    x.set(\gate, 0);
    // shut down the server process
    s.quit;

## Now type it again

Type the core ADSR definition and its lifecycle commands.

    // compile a pad definition
    SynthDef(\pad, { arg freq = 440, gate = 1;
        // local variables for signal and envelope
        var sig, env;
        // default ADSR driven by the gate
        env = EnvGen.kr(Env.adsr(), gate, doneAction: 2);
        // a square pulse shaped by the envelope
        sig = Pulse.ar(freq, 0.5) * env * 0.1;
        // send the signal to the hardware output
        Out.ar(0, sig);
    // send the definition to the server
    }).add;

    // spawn the pad and keep its reference
    x = Synth(\pad);
    // close the gate to begin the release phase
    x.set(\gate, 0);

## Wrap up

Env blueprint -> EnvGen.kr -> doneAction: 2 -> gate argument -> x.set(\gate, 0)
