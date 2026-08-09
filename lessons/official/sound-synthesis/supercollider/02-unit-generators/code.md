# Unit generators - typing

This lesson types the core oscillator generators, their keyword arguments, and
multi-channel expansion.

## Boot the server

An active server is required before any generator can be instantiated.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Ready for synthesis.");
    });

## The sine oscillator

SinOsc produces a pure tone with no overtones.

    s.waitForBoot({
        // create a node and hold its reference
        x = {
            // 440 hertz, phase zero, amplitude 0.1
            SinOsc.ar(440, 0, 0.1);
        }.play;
    });

## The sawtooth oscillator

Saw produces a bright spectrum of all integer harmonics.

    // free the previous node before starting a new one
    x.free;
    // create a new node and hold its reference
    x = {
        // 110 hertz at 0.1 amplitude
        Saw.ar(110, 0.1);
    }.play;

## The pulse oscillator

Pulse needs a width argument to set the duty cycle.

    // free the previous node before starting a new one
    x.free;
    // create a new node and hold its reference
    x = {
        // 110 hertz, 0.5 width (square), 0.1 amplitude
        Pulse.ar(110, 0.5, 0.1);
    }.play;

## The mul and add keywords

Writing arguments as keywords makes the intent explicit.

    // free the previous node before starting a new one
    x.free;
    // create a new node and hold its reference
    x = {
        // frequency 440, phase 0, scaled by 0.1
        SinOsc.ar(freq: 440, phase: 0, mul: 0.1, add: 0);
    }.play;

## Multi-channel expansion

An array argument spawns one generator per element.

    // free the previous node before starting a new one
    x.free;
    // create a new node and hold its reference
    x = {
        // left channel 110 hertz, right channel 111 hertz
        Pulse.ar([110, 111], 0.5, 0.1);
    }.play;

## Termination

Free the final node and quit the server to release the hardware.

    // free the running node
    x.free;
    // shut down the server process
    s.quit;

## Now type it again

Type the oscillator instantiations once more.

    // a 440 hertz sine at 0.1 amplitude
    x = { SinOsc.ar(440, 0, 0.1) }.play;
    // free the node
    x.free;
    // a 110 hertz sawtooth at 0.1 amplitude
    x = { Saw.ar(110, 0.1) }.play;
    // free the node
    x.free;
    // keyword arguments: frequency, phase, mul, add
    x = { SinOsc.ar(freq: 440, phase: 0, mul: 0.1, add: 0) }.play;
    // free the node
    x.free;
    // a pulse pair at 110 and 111 hertz
    x = { Pulse.ar([110, 111], 0.5, 0.1) }.play;
    // free the node
    x.free;

## Wrap up

Server boot -> sine -> saw -> pulse -> keywords -> array expansion -> quit
