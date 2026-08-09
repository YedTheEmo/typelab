# Amplitude modulation - typing

This lesson types amplitude modulation: low-rate tremolo, audio-rate ring
modulation, and unipolar scaling that preserves the carrier.

## Boot the server

Signal multiplication happens per sample on the server.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for amplitude modulation.");
    });

## Low frequency tremolo

A 6 hertz modulator makes the carrier's volume pulse.

    s.waitForBoot({
        // compile the tremolo definition
        SynthDef(\tremolo, { arg carFreq = 220, modFreq = 6, amp = 0.5;
            // locals for carrier, modulator, and envelope
            var car, mod, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 3.0), doneAction: 2);
            // a sawtooth as the carrier
            car = Saw.ar(carFreq);
            // a control-rate sine at 6 hertz
            mod = SinOsc.kr(modFreq);
            // multiply carrier by modulator, then shape
            Out.ar(0, (car * mod * env * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Ring modulation

At audio rates, bipolar multiplication yields sidebands only.

    s.waitForBoot({
        // compile the ring modulation definition
        SynthDef(\ringMod, { arg carFreq = 400, modFreq = 500, amp = 0.5;
            // locals for carrier, modulator, and envelope
            var car, mod, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 2.0), doneAction: 2);
            // a sine as the carrier
            car = SinOsc.ar(carFreq);
            // an audio-rate sine as the modulator
            mod = SinOsc.ar(modFreq);
            // multiply carrier by modulator, then shape
            Out.ar(0, (car * mod * env * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Unipolar amplitude modulation

Offsetting the modulator keeps the carrier in the output.

    s.waitForBoot({
        // compile the standard AM definition
        SynthDef(\standardAM, { arg carFreq = 400, modFreq = 500, amp = 0.5;
            // locals for carrier, modulator, and envelope
            var car, mod, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 2.0), doneAction: 2);
            // a sine as the carrier
            car = SinOsc.ar(carFreq);
            // shift the modulator range to 0.0-1.0
            mod = SinOsc.ar(modFreq, mul: 0.5, add: 0.5);
            // multiply carrier by the unipolar modulator
            Out.ar(0, (car * mod * env * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Execution

Spawn all three definitions and compare the results.

    // low-rate tremolo
    Synth(\tremolo);
    // audio-rate ring modulation
    Synth(\ringMod);
    // unipolar standard AM
    Synth(\standardAM);
    // shut down the server process
    s.quit;

## Now type it again

Type the unipolar scaling logic; the offset saves the carrier.

    // shift the modulator range to 0.0-1.0
    mod = SinOsc.ar(modFreq, mul: 0.5, add: 0.5);
    // a sine as the carrier
    car = SinOsc.ar(carFreq);
    // multiply carrier by the unipolar modulator
    sig = car * mod;

## Wrap up

Carrier -> Modulator -> bipolar (RM) vs unipolar (AM) -> multiplication
