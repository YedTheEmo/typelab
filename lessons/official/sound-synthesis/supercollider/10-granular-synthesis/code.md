# Granular synthesis - typing

This lesson types granular synthesis: an impulse trigger, grain windows, and a
position pointer sweeping through a buffer.

## Boot the server and load a buffer

Granular processing needs server-side memory for the source audio.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // load the sample file into server memory
        b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav");
        // report readiness
        postln("Buffer allocated for granular slicing.");
    });

## The granular graph

GrainBuf slices the buffer into overlapping grains.

    s.waitForBoot({
        // compile the granular definition
        SynthDef(\granular, { arg bufnum, rate = 1, pos = 0.5, amp = 0.5;
            // locals for trigger, signal, and grain duration
            var trig, sig, dur;
            // trigger grains 40 times per second
            trig = Impulse.kr(40);
            // each grain lasts 0.1 seconds
            dur = 0.1;
            // read grains from the buffer
            sig = GrainBuf.ar(
                // stereo output
                numChannels: 2,
                // the trigger source
                trigger: trig,
                // the grain duration
                dur: dur,
                // the source buffer
                sndbuf: bufnum,
                // playback rate
                rate: rate,
                // position in the buffer
                pos: pos,
                // linear interpolation
                interp: 2,
                // center panning
                pan: 0
            );
            // scale and send to the output
            Out.ar(0, sig * amp);
        // compile the graph and send it to the server
        }).add;
    });

## Dynamic traversal

A Line generator sweeps the position pointer across the buffer.

    s.waitForBoot({
        // compile the sweeping granular definition
        SynthDef(\granularSweep, { arg bufnum, amp = 0.5;
            // locals for trigger, signal, and position envelope
            var trig, sig, posEnv;
            // trigger grains 60 times per second
            trig = Impulse.kr(60);
            // sweep the position across ten seconds, then free
            posEnv = Line.kr(0, 1, 10, doneAction: 2);
            // read grains while the position moves
            sig = GrainBuf.ar(2, trig, 0.05, bufnum, 0.5, posEnv);
            // scale and send to the output
            Out.ar(0, sig * amp);
        // compile the graph and send it to the server
        }).add;
    });

## Execution and cleanup

The position Line frees the node; the buffer is freed separately.

    // spawn the sweeping granular node
    Synth(\granularSweep, [\bufnum, b.bufnum]);

    // schedule cleanup eleven seconds later
    SystemClock.sched(11.0, {
        // release the buffer from server memory
        b.free;
        // shut down the server process
        s.quit;
        // return nil to avoid repeating the function
        nil;
    });

## Now type it again

Type the GrainBuf structure; trigger, duration, and position are independent.

    // trigger grains 60 times per second
    trig = Impulse.kr(60);
    // sweep the position across ten seconds, then free
    posEnv = Line.kr(0, 1, 10, doneAction: 2);
    // read grains while the position moves
    sig = GrainBuf.ar(2, trig, 0.05, bufnum, 0.5, posEnv);

## Wrap up

Buffer memory -> Impulse trigger -> GrainBuf windowing -> parameter decoupling
