# Synthesis definitions - typing

This lesson types the difference between a SynthDef blueprint and a running
Synth node: compile, instantiate, override arguments, and modify live nodes.

## Boot the server

Definitions are compiled and sent to the server after boot.

    // assign the default local server to a global variable
    s = Server.default;
    // wait for the server to finish booting
    s.waitForBoot({
        // report readiness
        postln("Server is operational.");
    });

## Compiling the definition

A SynthDef is a named blueprint with default arguments.

    s.waitForBoot({
        // compile the definition named basicPulse
        SynthDef(\basicPulse, { arg freq = 220, amp = 0.1, width = 0.5;
            // local variable for the signal
            var sig;
            // a pulse oscillator using the three arguments
            sig = Pulse.ar(freq, width, amp);
            // send the signal to the hardware output
            Out.ar(0, sig);
        // compile the graph and send it to the server
        }).add;
    });

## Instantiation

The blueprint produces nothing until a node is spawned.

    // spawn a node running the basicPulse definition
    x = Synth(\basicPulse);

## Passing initial arguments

Arguments can be overridden at the moment of instantiation.

    // free the previous node
    x.free;
    // spawn a node with lower frequency and a narrower pulse
    x = Synth(\basicPulse, [\freq, 110, \width, 0.2, \amp, 0.15]);

## Modifying active nodes

A running node updates its control inputs through set messages.

    // drop the frequency to 55 hertz
    x.set(\freq, 55);
    // widen the pulse to 0.8
    x.set(\width, 0.8);
    // set several parameters in one message
    x.set(\freq, 220, \amp, 0.05);

## Termination

Free the node and quit the server to stop audio work.

    // free the running node
    x.free;
    // shut down the server process
    s.quit;

## Now type it again

Type the sequence from compilation to manipulation.

    // compile a simple tone definition
    SynthDef(\testTone, { arg freq = 440, amp = 0.1;
        // a sine oscillator scaled by the amplitude
        var sig = SinOsc.ar(freq, 0, amp);
        // send the signal to the hardware output
        Out.ar(0, sig);
    // send the definition to the server
    }).add;

    // spawn the tone at 330 hertz
    x = Synth(\testTone, [\freq, 330]);
    // raise the frequency to 440 hertz
    x.set(\freq, 440);
    // raise the amplitude to 0.2
    x.set(\amp, 0.2);
    // free the running node
    x.free;

## Wrap up

SynthDef -> args -> Out.ar -> add -> Synth -> set -> free
