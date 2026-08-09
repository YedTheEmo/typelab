# Buffers - concepts

Assuming the client application can read an audio file from a hard drive and pass
the data stream over the network to the server is a fatal architectural error.
Sclang cannot process audio files. All audio data must reside in server-side RAM
before it can be manipulated. You facilitate this using Buffers.

A Buffer is a contiguous block of memory allocated within the scsynth process.
When you load an audio file, you instruct the server to read the file from disk
and write its sample data into one of these memory blocks. The client retains
only an integer identifier, the buffer number, serving as a remote pointer.

Loading a file into memory involves mechanical disk latency. It is an asynchronous
operation. If you instruct the server to load a file and immediately execute a
synth node referencing that buffer on the very next line of code, the node will
fail. The disk read will not have finished. You must manage this asynchronous
timeline explicitly.

    b = Buffer.read(s, "/path/to/file.wav");

This allocates a buffer and initiates the disk read. To utilize the stored data,
you construct a SynthDef using the PlayBuf unit generator. PlayBuf does not take
a file path; it takes the integer index of the allocated buffer.

    PlayBuf.ar(1, bufnum: 0);

The first argument defines the number of channels the buffer contains. This must
be hardcoded during compilation. A SynthDef cannot dynamically adjust its channel
architecture after it is compiled. If you compile a 1-channel PlayBuf and pass it
a 2-channel stereo buffer, the engine will truncate the data and output only the
left channel.

Playback speed requires explicit scaling. A buffer recorded at 44.1kHz played back
on a 48kHz server will mathematically drift, sounding pitched-up and sped-up. You
must multiply the playback rate by the BufRateScale generator to force the server
to dynamically calculate the sample rate ratio and compensate.

    PlayBuf.ar(1, 0, rate: BufRateScale.kr(0));

When you are finished with an audio file, it remains in the server's RAM until
the process terminates. You must explicitly free the buffer memory to prevent
eventual system starvation.

## Next step

Now type the code version of this lesson.
