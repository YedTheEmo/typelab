# Amplitude modulation - concepts

While frequency modulation involves adding a signal to an oscillator's pitch
parameter, amplitude modulation (AM) is strictly a multiplication operation. You
route an audio signal through a multiplier that is controlled by another
oscillator. The mathematical reality of multiplying two waves produces distinct
acoustic phenomena depending on the modulation rate.

When the modulating oscillator operates at sub-audio rates (below 20 hertz), human
perception tracks the amplitude changes dynamically. We hear the volume of the
carrier signal rising and falling.

    sig = SinOsc.ar(440) * SinOsc.ar(5);

This yields a standard tremolo effect, oscillating five times per second. However,
assuming this behavior holds at higher frequencies is incorrect. When the modulator
crosses the audio threshold, our ears cease to hear volume fluctuations. Instead,
the multiplication generates sidebands.

Specifically, bipolar multiplication (where the modulator swings between 1.0 and
-1.0) produces Ring Modulation. Ring Modulation mathematically destroys the carrier
frequency and the modulator frequency, outputting only their sum and difference.
If a 400 hertz carrier is ring-modulated by a 100 hertz signal, the output
contains only 300 hertz and 500 hertz. The fundamental frequency vanishes,
yielding a hollow, metallic dissonance.

Standard Amplitude Modulation requires unipolar modulation. To preserve the
original carrier frequency alongside the sidebands, the modulating signal must
swing strictly between 0.0 and 1.0. You achieve this by scaling the modulator
using the mul and add arguments before the multiplication occurs.

    mod = SinOsc.ar(100, mul: 0.5, add: 0.5);

By forcing the modulator into a positive unipolar range, the multiplication
scales the carrier rather than inverting it. This outputs the sum, the difference,
and the original carrier frequency, creating a denser, thicker spectrum than
Ring Modulation.

## Next step

Now type the code version of this lesson.
