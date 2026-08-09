# Subtractive synthesis - typing

This lesson types subtractive synthesis: filtering noise, sweeping a cutoff,
and scaling a rich source into a narrower spectrum.

## Boot the server

Subtractive synthesis relies on control-rate modulation.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for filters.");
    });

## Static filtering

A band-pass filter isolates one slice of a noise spectrum.

    s.waitForBoot({
        // compile the noise sweep definition
        SynthDef(\noiseSweep, { arg freq = 1000, rq = 0.1, amp = 0.5;
            // locals for the filtered signal and the envelope
            var sig, env;
            // percussive envelope that frees the node
            env = EnvGen.kr(Env.perc(0.01, 1.0), doneAction: 2);
            // white noise through a band-pass filter
            sig = BPF.ar(WhiteNoise.ar(1.0), freq, rq);
            // shape, scale, and duplicate to both channels
            Out.ar(0, (sig * env * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Dynamic cutoff modulation

A control-rate envelope sweeps the filter cutoff over time.

    s.waitForBoot({
        // compile the sub bass definition
        SynthDef(\subBass, { arg freq = 55, gate = 1, amp = 0.5;
            // locals for signal, amplitude, and filter envelopes
            var sig, ampEnv, filterEnv;
            // amplitude ADSR driven by the gate
            ampEnv = EnvGen.kr(Env.adsr(0.01, 0.3, 0.6, 1.0), gate, doneAction: 2);
            // exponential cutoff drop from 8000 to 100 hertz
            filterEnv = EnvGen.kr(Env([8000, 8000, 100], [0.01, 0.5], \exp));
            // a sawtooth as the rich harmonic source
            sig = Saw.ar(freq);
            // resonant low-pass filter driven by the sweep
            sig = RLPF.ar(sig, filterEnv, 0.2);
            // shape, scale, and duplicate to both channels
            Out.ar(0, (sig * ampEnv * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Execution

Spawn the noise bursts and the swept bass.

    // a noise band centered at 400 hertz
    Synth(\noiseSweep, [\freq, 400]);
    // a noise band centered at 3000 hertz
    Synth(\noiseSweep, [\freq, 3000]);

    // spawn the swept bass and keep its reference
    x = Synth(\subBass, [\freq, 41.2]);
    // close the gate to begin the release phase
    x.set(\gate, 0);
    // shut down the server process
    s.quit;

## Now type it again

Type the core dynamic filter graph.

    // exponential cutoff drop from 5000 to 200 hertz
    filterEnv = EnvGen.kr(Env([5000, 200], [0.4], \exp));
    // a sawtooth as the rich harmonic source
    sig = Saw.ar(freq);
    // resonant low-pass filter driven by the sweep
    sig = RLPF.ar(sig, filterEnv, 0.1);

## Wrap up

Rich harmonic source -> LPF/HPF/BPF -> resonance scaling -> cutoff modulation
