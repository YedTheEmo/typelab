# SuperCollider architecture - concepts

Standard audio programming often assumes a monolithic structure where the interface
and the digital signal processor run in the same memory space. SuperCollider
invalidates this assumption by enforcing a strict bifurcation. The system operates
as two distinct applications communicating over a network protocol.

The first component is the client application, sclang. This is an interpreter
running an object-oriented language. It handles all logical operations, algorithmic
generation, and user interfaces. It produces absolutely no sound.

The second component is the server application, scsynth. This is a highly optimized
digital signal processing engine. It executes graph structures to calculate audio
samples, but it contains no internal logic for composition or interface handling.

The assumption that local execution bypasses network constraints is false. Even on
a single machine, sclang and scsynth communicate strictly via Open Sound Control
messages over UDP or TCP. This network dependency introduces mandatory latency. A
command sent from the client application takes time to reach the audio engine.
This allows forward-thinking implementations where sclang runs on one machine
while scsynth processes audio on a dedicated remote computing cluster.

This bifurcation forces a skeptical approach to variable scope. A variable defined
in sclang memory cannot be read by scsynth. Data must be explicitly passed as
control parameters across the network boundary. To generate audio, you must first
boot the server to launch the process and establish the connection.

    s = Server.default;
    s.boot;

The variable s is a global identifier representing the default local server.
You cannot send audio commands before the server finishes booting. Doing so results
in failed memory allocations. You must wait for the hardware initialization to
complete by registering a callback function.

    s.waitForBoot({
        postln("Server is operational.");
    });

Once booted, the client sends messages to the server to construct audio nodes.
A simple test function verifies the signal path.

    { SinOsc.ar(440, 0, 0.1) }.play;

This method is a client-side abstraction. It constructs a definition object behind
the scenes, assigns a randomized name, sends the definition byte-code to the server,
and then sends an immediate message to instantiate it. Relying on this abstraction
masks the true asynchronous reality of the system, but it serves initial testing.

When the session concludes, you must explicitly terminate the server process to
free the audio hardware and system memory. Failure to do so leaves background
processes consuming system resources indefinitely.

    s.quit;

## Next step

Now type the code version of this lesson.
