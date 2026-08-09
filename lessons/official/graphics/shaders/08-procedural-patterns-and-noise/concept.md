# Procedural patterns and noise - concepts

A shader does not need an image, texture, or collection of stored values to
produce variation. It can derive a value directly from the coordinates of
each pixel.

This is the foundation of procedural graphics. A coordinate enters a
mathematical function, and the function produces a value that varies across
space. The shader can then use that value for shape, color, displacement,
opacity, or almost any other visual property.

The challenge is producing variation that is useful. A simple sine wave is
continuous and predictable, but its regularity is often too obvious for
natural-looking patterns. Noise techniques introduce controlled irregularity
while preserving useful mathematical properties such as continuity.

## A coordinate is already an input

Every pixel has a position, and that position can be treated as the input to a
function.

```slang id="7m2vke"
float value = sin(p.x * frequency);
```

The value changes as x changes, creating a repeating pattern.

This is already procedural generation. Nothing was stored beforehand. The
pattern exists because the same equation is evaluated independently at every
coordinate.

Procedural noise uses the same principle, but instead of deliberately
repeating a simple wave, it derives values that appear irregular while
remaining mathematically reproducible.

## Why randomness is not enough

A random number generator could produce an unpredictable value for every
pixel, but independent randomness usually produces static-looking grain.

```slang id="x9f5j2"
float value = random();
```

Adjacent pixels have no reason to receive similar values. For many visual
effects, that is undesirable because natural surfaces and useful procedural
patterns usually change gradually rather than jumping randomly from one pixel
to the next.

Noise therefore needs more than randomness. It needs a relationship between
nearby coordinates.

The desired property is usually continuity: small changes in the input should
produce small changes in the output.

## Hash functions create deterministic variation

A hash function maps an input coordinate to a seemingly irregular value while
always returning the same result for the same input.

A simple two-dimensional hash can be constructed from a dot product and sine:

```slang id="1k2z7n"
float hash(float2 p) {
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}
```

The dot product combines the two coordinates into one scalar. Sine scrambles
the result into a rapidly changing sequence, and `frac` keeps only the
fractional portion.

The large constants are not magical values that describe a particular object.
They simply help produce a useful distribution of values across different
coordinates.

The important property is deterministic variation. The same input always
produces the same output, so every pixel can independently reproduce its value.

## Random values at lattice points

A useful noise strategy is to generate random values only at discrete points
of a grid.

For a coordinate `p`, separate its integer cell position from its fractional
position.

```slang id="5w3s8j"
float2 cell = floor(p);
float2 local = frac(p);
```

The integer part identifies the current grid cell. The fractional part tells us
where the point lies inside that cell.

A hash can then produce one deterministic value for each cell:

```slang id="n7c6xq"
float corner = hash(cell);
```

Every point inside the same cell receives the same corner value. That is not
yet useful noise because the result has abrupt boundaries, but it gives us the
raw material needed to construct continuous variation.

## Interpolation removes the hard boundaries

Instead of using the value from one grid point everywhere inside its cell,
interpolate between neighboring grid values.

In one dimension, linear interpolation is:

```slang id="h4y9pz"
float value = lerp(a, b, t);
```

When `t` is zero, the result is `a`. When `t` is one, the result is `b`.
Intermediate values smoothly transition between them.

For a two-dimensional cell, four corner values contribute to the result.

```slang id="0w4f8x"
float a = hash(cell);
float b = hash(cell + float2(1.0, 0.0));
float c = hash(cell + float2(0.0, 1.0));
float d = hash(cell + float2(1.0, 1.0));
```

These four values define the corners of the current cell. The fractional
coordinate determines how much influence each corner should have.

## Smooth interpolation

Linear interpolation is continuous, but its rate of change can abruptly
change at the boundaries between cells.

A smooth interpolation curve can make the transition less mechanically
obvious.

One common smoothstep curve is:

```slang id="q2m8ds"
float2 u = local * local * (3.0 - 2.0 * local);
```

The values still move from zero to one, but the curve eases into and out of
the endpoints.

The resulting noise has smoother gradients because neighboring cells meet
with matching first-order behavior.

This is an important distinction between random values and noise. The random
values are the raw material. Interpolation is what turns those values into a
continuous field.

## Bilinear interpolation

The four corner values can be interpolated first along x and then along y.

```slang id="6g8t1a"
float x1 = lerp(a, b, u.x);
float x2 = lerp(c, d, u.x);
float value = lerp(x1, x2, u.y);
```

The first two interpolations move horizontally between the corners. The final
interpolation moves vertically between those horizontal results.

The output is now a continuous scalar field across the cell.

The exact implementation can vary, but the mathematical idea remains the
same: nearby lattice values influence the region between them.

## A basic value-noise function

The previous operations can be assembled into one reusable function.

```slang id="2r7m1k"
float noise(float2 p) {
    float2 cell = floor(p);
    float2 local = frac(p);
    float2 u = local * local * (3.0 - 2.0 * local);

    float a = hash(cell);
    float b = hash(cell + float2(1.0, 0.0));
    float c = hash(cell + float2(0.0, 1.0));
    float d = hash(cell + float2(1.0, 1.0));

    float x1 = lerp(a, b, u.x);
    float x2 = lerp(c, d, u.x);

    return lerp(x1, x2, u.y);
}
```

The function accepts any continuous coordinate and returns a smoothly varying
value.

The grid itself is no longer visible because the interpolation hides the
individual corner values behind a continuous field.

## Scale controls the size of the pattern

Noise is not inherently large or small. The scale of its input determines
how quickly its value changes through space.

```slang id="q1f3vj"
float value = noise(p * 4.0);
```

A larger multiplier causes the noise field to change more rapidly, producing
smaller features. A smaller multiplier stretches the same field across a
larger region.

This is the same frequency principle encountered with sine-based domain
warping. Input scaling controls spatial frequency.

That means noise can be treated like any other mathematical function whose
domain can be transformed.

## Combining multiple scales

One noise layer often looks too uniform. A common technique is to evaluate
noise at several frequencies and combine the results.

```slang id="w6z2qa"
float value = noise(p * 2.0);
value += noise(p * 4.0) * 0.5;
value += noise(p * 8.0) * 0.25;
```

The first layer contributes broad variation. Higher-frequency layers add
smaller details on top.

The decreasing multipliers prevent the fine layers from overwhelming the
large-scale structure.

This produces a more complex field without requiring a more complicated
primitive. Complexity emerges from combining simple fields at different
scales.

## Fractional Brownian motion

This repeated combination is commonly called fractional Brownian motion, or
fBm.

A typical structure is:

```slang id="p6g1kd"
float value = 0.0;
float amplitude = 0.5;

value += noise(p);
value += noise(p * 2.0) * amplitude;
value += noise(p * 4.0) * amplitude * 0.5;
```

The exact constants are adjustable. The important pattern is increasing
frequency combined with decreasing amplitude.

Each octave contributes a different spatial scale. Low frequencies establish
the broad structure, while high frequencies supply detail.

This same idea appears in clouds, terrain, smoke, marble, wood-like patterns,
and many other procedural materials.

## Noise is a field, not a texture

A useful mental model is to treat noise as a scalar field.

At every coordinate, the function answers one question: what value exists here?

```slang id="x5q2ra"
float value = noise(p);
```

That value does not have to become a grayscale pixel. It can control another
mathematical operation.

For example, it can displace a coordinate:

```slang id="c4n8yt"
float distortion = noise(p * 3.0);
float2 q = p + distortion * 0.1;
```

The noise becomes a source of domain warping.

It can also modify a shape:

```slang id="a8k4ye"
float radius = 0.3 + noise(p * 5.0) * 0.05;
```

Now the radius changes across the domain, producing an irregular boundary.

## Thresholding a scalar field

A continuous field can be converted into regions by comparing it against a
threshold.

```slang id="s9r5zc"
float value = noise(p * 4.0);
float mask = step(0.5, value);
```

Values below the threshold become one side of the mask and values above it
become the other.

This turns smooth noise into a binary pattern.

A hard threshold can look harsh, so a smooth threshold is often more useful:

```slang id="e3h7kw"
float mask = smoothstep(0.4, 0.6, value);
```

Now values near the boundary transition gradually.

This is another example of separating mathematical stages. Noise generates the
field, while the threshold determines how that field is interpreted.

## Noise and domain warping

Noise becomes especially powerful when it controls the coordinates of another
function.

```slang id="v8q2nm"
float distortion = noise(p * 3.0);
float2 q = p + distortion * 0.15;
float value = sin(q.x * 12.0);
```

The noise creates a continuously varying displacement. The sine pattern then
samples the warped domain.

This combines the techniques from the previous lesson with the new concept
introduced here. Domain warping determines how coordinates move, while noise
provides an irregular but continuous source for that movement.

## The deeper mental model

Procedural noise is best understood as a pipeline rather than a special visual
effect.

First, deterministic hashing creates irregular values at known locations.
Interpolation turns those discrete values into a continuous field. Input
scaling controls the size of its features. Multiple scales can then be
combined to create richer structure.

The resulting field can drive almost anything:

```slang id="v9x6bp"
float value = noise(p);
```

The value might become color, displacement, a threshold, a radius, a lighting
parameter, or another domain transformation.

That is why procedural noise is so useful in shader work. It is not primarily a
texture replacement. It is a mathematical source of spatial variation that
can be composed with the rest of the shader's functions.

## Next step

Now type the code version of this lesson.

