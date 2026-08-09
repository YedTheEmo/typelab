# Frequency modulation - typing

This lesson types FM synthesis: a modulator offsetting the carrier's frequency,
and an envelope that sweeps the modulation index.

## Boot the server

FM graphs require precise mathematical routing after boot.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for frequency modulation.");
    });

## Static FM routing

The modulator's output is added into the carrier's frequency.

    s.waitForBoot({
        // compile the basic FM definition
        SynthDef(\basicFM, { arg carFreq = 440, modFreq = 220, modIndex = 500, amp = 0.2;
            // locals for carrier, modulator, and envelope
            var car, mod, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 2.0), doneAction: 2);
            // scale the modulator by the index
            mod = SinOsc.ar(modFreq) * modIndex;
            // add the modulator into the carrier frequency
            car = SinOsc.ar(carFreq + mod);
            // shape, scale, and duplicate to both channels
            Out.ar(0, (car * env * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Dynamic index modulation

An envelope on the index makes the timbre morph over time.

    s.waitForBoot({
        // compile the dynamic FM definition
        SynthDef(\dynamicFM, { arg carFreq = 440, modRatio = 1.5, indexMax = 1000, amp = 0.2;
            // locals for signals, envelopes, and the derived modulator
            var car, mod, ampEnv, indexEnv, modFreq;
            // derive the modulator frequency from the ratio
            modFreq = carFreq * modRatio;
            // percussive envelope that frees the node
            ampEnv = EnvGen.kr(Env.perc(0.01, 3.0), doneAction: 2);
            // exponential index decay from indexMax to zero
            indexEnv = EnvGen.kr(Env([indexMax, 0], [2.0], \exp));
            // scale the modulator by the moving index
            mod = SinOsc.ar(modFreq) * indexEnv;
            // add the modulator into the carrier frequency
            car = SinOsc.ar(carFreq + mod);
            // shape, scale, and duplicate to both channels
            Out.ar(0, (car * ampEnv * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Execution

Spawn both patches to hear static and morphing timbres.

    // a harmonic 1:2 ratio at a strong index
    Synth(\basicFM, [\carFreq, 300, \modFreq, 600, \modIndex, 800]);

    // an inharmonic 1:1.3 ratio with a high starting index
    Synth(\dynamicFM, [\carFreq, 200, \modRatio, 1.3, \indexMax, 2000]);
    // shut down the server process
    s.quit;

## Now type it again

Type the core oscillator routing; the modulator offsets the carrier.

    // scale the modulator by the index
    mod = SinOsc.ar(modFreq) * modIndex;
    // add the modulator into the carrier frequency
    car = SinOsc.ar(carFreq + mod);

## Wrap up

Modulator wave -> multiply by index -> add to carrier freq -> generate sidebands
