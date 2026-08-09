# Additive synthesis - concepts

Fourier theory dictates that any complex waveform can be mathematically deconstructed
into a sum of mathematically pure sine waves at various frequencies, amplitudes,
and phases. Additive synthesis reverses this analysis. It assumes that by summing
multiple sine wave generators, you can construct any acoustic timbre.

In a digital environment, summing signals is literal mathematical addition. If
you generate two audio streams and add them together, the server merges them into
a single continuous stream.

    sig = SinOsc.ar(440) + SinOsc.ar(880);

While this manual addition functions for two oscillators, scaling it to dozens
of partials becomes syntactically unmanageable. SuperCollider addresses this
using arrays and the Mix object. When you feed an array of unit generators into
a Mix object, it automatically collapses the multi-channel array down to a single
monophonic audio stream by summing all internal elements.

    sig = Mix.new([SinOsc.ar(440), SinOsc.ar(880), SinOsc.ar(1320)]);

A critical failure point in additive synthesis is amplitude accumulation. The
digital threshold for a standard audio interface is exactly 1.0 or -1.0. If you
sum ten sine waves, each with an amplitude of 0.2, the peak output will hit 2.0.
The hardware will violently truncate the waveform, causing severe digital clipping.

Assuming the server will automatically scale your output to prevent clipping
is a structural error. You must explicitly scale the final mixed signal down
by dividing it by the number of partials, or by applying a fractional multiplier.

To rapidly generate dense arrays of generators, we utilize algorithmic iteration
rather than manual typing. The Mix.fill method executes a function a specified
number of times, passes the iteration index as an argument, and collects the
resulting unit generators into a summed output.

    sig = Mix.fill(8, { arg index; SinOsc.ar(440 * (index + 1)) });

This approach allows forward-thinking programmatic generation of massive spectral
structures with minimal code, strictly enforcing structural parameters over
manual labor.

## Next step

Now type the code version of this lesson.
