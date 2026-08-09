# Additive synthesis - typing

This lesson types additive synthesis: manual addition of oscillators, array
mixing, and algorithmic generation with Mix.fill.

## Boot the server

Additive arrays compile into complex graphs, so boot first.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for additive structures.");
    });

## Manual addition

Three oscillators are summed to build an organ tone.

    s.waitForBoot({
        // compile the manual organ definition
        SynthDef(\manualOrgan, { arg freq = 440, gate = 1;
            // locals for the three voices, the sum, and the envelope
            var sig1, sig2, sig3, sum, env;
            // amplitude envelope driven by the gate
            env = EnvGen.kr(Env.adsr(), gate, doneAction: 2);
            // the fundamental
            sig1 = SinOsc.ar(freq);
            // the second harmonic
            sig2 = SinOsc.ar(freq * 2);
            // the third harmonic
            sig3 = SinOsc.ar(freq * 3);
            // add the three voices together
            sum = sig1 + sig2 + sig3;
            // scale by 0.3 and duplicate to both channels
            Out.ar(0, (sum * env * 0.3) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Array mixing

Mix collapses an array of signals into one stream.

    s.waitForBoot({
        // compile the array organ definition
        SynthDef(\arrayOrgan, { arg freq = 440, gate = 1;
            // locals for the mixed signal and the envelope
            var sig, env;
            // amplitude envelope driven by the gate
            env = EnvGen.kr(Env.adsr(), gate, doneAction: 2);
            // mix the three harmonic voices into one signal
            sig = Mix.new([
                SinOsc.ar(freq),
                SinOsc.ar(freq * 2),
                SinOsc.ar(freq * 3)
            ]);
            // scale by 0.3 and duplicate to both channels
            Out.ar(0, (sig * env * 0.3) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Algorithmic generation

Mix.fill builds a dense bell timbre from twelve random partials.

    s.waitForBoot({
        // compile the bell definition
        SynthDef(\bell, { arg freq = 200, amp = 1;
            // locals for the mixed signal and the envelope
            var sig, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 3.0), doneAction: 2);
            // generate twelve partials, summed automatically
            sig = Mix.fill(12, { arg index;
                // randomize each partial around the base frequency
                var partialFreq = freq * rand2(1.0, 5.0).abs;
                // a sine at that random partial frequency
                SinOsc.ar(partialFreq)
            });
            // divide by 12 for safe headroom and go stereo
            Out.ar(0, (sig * env * amp / 12) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Execution

Spawn the compiled definitions to hear the structures.

    // spawn the array organ at 330 hertz and keep its reference
    x = Synth(\arrayOrgan, [\freq, 330]);
    // close the gate to begin the release phase
    x.set(\gate, 0);

    // spawn the bell at 400 hertz
    Synth(\bell, [\freq, 400]);
    // shut down the server process
    s.quit;

## Now type it again

Type the Mix.fill structure; the index argument drives the harmonics.

    // generate eight harmonic partials and sum them
    sig = Mix.fill(8, { arg i;
        // scale the fundamental and fade each partial
        SinOsc.ar(freq * (i + 1)) * (1 / (i + 1))
    });

## Wrap up

Array of generators -> Mix.new -> Mix.fill(n, { ... }) -> amplitude scaling
