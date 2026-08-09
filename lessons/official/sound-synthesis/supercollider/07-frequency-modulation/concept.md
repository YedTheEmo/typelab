# Frequency modulation - concepts

Additive and subtractive techniques rely on linear operations. Frequency
Modulation (FM) is a non-linear process. You create complex spectra not by adding
discrete harmonics, but by utilizing one unit generator to explicitly modulate
the frequency parameter of another at audio rates.

The architecture demands two oscillators. The primary oscillator is the Carrier.
This is the unit generator that routes its output directly to the audio hardware.
The secondary oscillator is the Modulator. Its output is never heard directly;
instead, its output is mathematically added to the frequency argument of the Carrier.

    mod = SinOsc.ar(100) * 200;
    car = SinOsc.ar(400 + mod);

In this example, the Modulator oscillates at 100 hertz. The multiplier of 200
is the Modulation Index. It dictates the amplitude of the modulator. Because the
modulator is added to the 400 hertz base frequency of the Carrier, the Carrier's
actual pitch swings wildly between 200 hertz and 600 hertz, one hundred times
per second.

Assuming this rapid fluctuation sounds like vibrato is incorrect. When modulation
occurs at audio rates (above 20 hertz), human perception breaks down. We cease
to hear individual pitch variations and instead hear new, distinct frequencies
called sidebands. The Carrier and Modulator synthesize a dense, metallic timbre.

The spectral outcome is strictly governed by the mathematical ratio between the
Carrier frequency and the Modulator frequency. If the ratio is a simple integer
(e.g., the Carrier is 400 hertz and the Modulator is 800 hertz, a 1:2 ratio), the
resulting sidebands align with the harmonic series. The sound resembles acoustic
instruments like bells or electric pianos.

If the ratio is irrational or fractional (e.g., a 1:1.414 ratio), the sidebands
fall outside the harmonic series. This produces inharmonic spectra, yielding
metallic, abrasive, or percussive sounds indistinguishable from gongs or noise.

The Modulation Index defines the bandwidth of these sidebands. A low index produces
a dull sound close to a pure sine wave. A massive index generates an aggressive,
wide spectrum. By applying an envelope strictly to the Modulation Index, you can
dynamically sculpt the timbral brightness over time without touching a filter.

## Next step

Now type the code version of this lesson.
