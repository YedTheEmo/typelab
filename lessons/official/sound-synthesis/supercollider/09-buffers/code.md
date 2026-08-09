# Buffers - typing

This lesson types sample playback: allocate a buffer, play it back with
PlayBuf, reverse it, and free the memory.

## Boot the server

Buffer operations need server-side memory.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for memory allocation.");
    });

## Allocating buffer memory

The server loads a file into its RAM.

    s.waitForBoot({
        // load the sample file into server memory
        b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav");
    });

## The playback definition

PlayBuf reads the buffer with rate-scaled playback.

    s.waitForBoot({
        // compile the sampler definition
        SynthDef(\sampler, { arg bufnum, rate = 1, amp = 0.5;
            // local variable for the signal
            var sig;
            // read the buffer with a scaled rate
            sig = PlayBuf.ar(
                // mono file
                numChannels: 1,
                // the buffer index passed at spawn time
                bufnum: bufnum,
                // preserve the file pitch across sample rates
                rate: BufRateScale.kr(bufnum) * rate,
                // free the node when playback ends
                doneAction: 2
            );
            // scale and duplicate to both channels
            Out.ar(0, (sig * amp) ! 2);
        // compile the graph and send it to the server
        }).add;
    });

## Instantiation

Spawn nodes against the loaded buffer index.

    // play the file forward at normal speed
    Synth(\sampler, [\bufnum, b.bufnum, \rate, 1.0]);

    // play the file in reverse at half speed
    Synth(\sampler, [\bufnum, b.bufnum, \rate, -0.5]);

## Deallocation

Free the buffer memory when playback is finished.

    // release the buffer from server memory
    b.free;
    // shut down the server process
    s.quit;

## Now type it again

Type the PlayBuf configuration; the channel count stays fixed.

    // read the buffer with a scaled rate
    sig = PlayBuf.ar(
        // mono file
        numChannels: 1,
        // the buffer index passed at spawn time
        bufnum: bufnum,
        // preserve the file pitch across sample rates
        rate: BufRateScale.kr(bufnum) * rate,
        // free the node when playback ends
        doneAction: 2
    );

## Wrap up

Buffer.read -> PlayBuf.ar -> BufRateScale.kr -> bufnum routing -> Buffer.free
