# SuperCollider architecture - typing

This lesson types the server lifecycle: define the server, boot it, create an
audio node, and shut the server down.

## Boot configuration

The server options set the hardware interface before booting.

    // assign the default local server to a global variable
    s = Server.default;
    // two output channels: left and right
    s.options.numOutputBusChannels = 2;
    // two input channels for audio capture
    s.options.numInputBusChannels = 2;
    // explicit sample rate avoids driver resampling
    s.options.sampleRate = 48000;

## Asynchronous boot procedure

Booting is non-blocking, so dependent code must wait in a callback.

    // wait for the server to finish booting
    s.waitForBoot({
        // report when the boot sequence completes
        postln("Boot process complete.");
    });

## Node instantiation

Inside the callback, the server allocates an audio node running a synthesis graph.

    s.waitForBoot({
        // create a node and hold its reference
        x = {
            // local variable for the signal
            var sig;
            // a 440 hertz sine at 0.1 amplitude
            sig = SinOsc.ar(440, 0, 0.1);
            // route the signal to the hardware output bus
            Out.ar(0, sig);
        }.play;
    });

## Server termination

An active server holds the audio hardware, so it must be told to quit.

    // schedule this function three seconds from now
    SystemClock.sched(3.0, {
        // free the audio node
        x.free;
        // shut down the server process
        s.quit;
        // report the shutdown
        postln("Server terminated.");
        // return nil to avoid repeating the function
        nil;
    });

## Now type it again

Type the complete server lifecycle script.

    // assign the default local server to a global variable
    s = Server.default;
    // two output channels: left and right
    s.options.numOutputBusChannels = 2;
    // explicit sample rate avoids driver resampling
    s.options.sampleRate = 48000;

    // wait for the server to finish booting
    s.waitForBoot({
        // create a node and hold its reference
        x = {
            // local variable for the signal
            var sig;
            // a 440 hertz sine at 0.1 amplitude
            sig = SinOsc.ar(440, 0, 0.1);
            // route the signal to the hardware output bus
            Out.ar(0, sig);
        }.play;

        // schedule the shutdown three seconds later
        SystemClock.sched(3.0, {
            // free the audio node
            x.free;
            // shut down the server process
            s.quit;
            // return nil to avoid repeating the function
            nil;
        });
    });

## Wrap up

Configure options -> wait for boot -> instantiate node -> free node -> quit server
