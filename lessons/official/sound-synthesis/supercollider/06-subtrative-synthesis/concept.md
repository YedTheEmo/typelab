# Subtractive synthesis - concepts

Additive synthesis requires intensive processing overhead to calculate independent
oscillators. Subtractive synthesis adopts the inverse strategy. It assumes you
begin with a densely harmonic audio source and deploy filters to remove specific
frequency bands, sculpting the final spectrum.

The required source material must be harmonically rich. A sine wave contains
only one fundamental frequency. Sending a sine wave through a filter is a futile
operation; there are no overtones to subtract. You must use broadband sources
like WhiteNoise, which contains all frequencies at equal amplitude, or Saw waves,
which contain all integer harmonics.

    sig = WhiteNoise.ar(0.5);

A filter is a specialized unit generator that attenuates frequencies beyond a
specific boundary. The Low Pass Filter (LPF) permits low frequencies to pass
unaltered while aggressively silencing frequencies above its cutoff parameter.

    sig = LPF.ar(WhiteNoise.ar(0.5), 1000);

This graph generates white noise but strictly suppresses spectral data above
one thousand hertz. The high frequencies are discarded.

Many digital filters include a resonance parameter, often denoted as rq
(reciprocal of quality factor) in SuperCollider. Resonance mathematically
amplifies the frequencies immediately surrounding the cutoff point before the
attenuation slope begins.

    sig = RLPF.ar(Saw.ar(100), 2000, 0.1);

A low rq value creates a sharp, pronounced resonant peak. A high rq value flattens
the peak. Assuming resonance is merely a tonal preference is a mistake. High
resonance causes extreme amplitude spikes at the cutoff frequency. This frequently
violates the digital threshold, leading to clipping and hardware damage if left
unscaled.

A static filter produces a static, uninteresting timbre. The defining characteristic
of subtractive sound design is the dynamic modulation of the cutoff frequency.
Because the cutoff argument accepts a control-rate input, we can route an EnvGen
directly into the filter's frequency parameter.

    env = EnvGen.kr(Env([100, 5000, 100], [0.5, 0.5]));
    sig = LPF.ar(Saw.ar(110), env);

This routes the mathematical output of the envelope to dynamically sweep the
filter open and closed, creating a classic brass or bass articulation.

## Next step

Now type the code version of this lesson.
