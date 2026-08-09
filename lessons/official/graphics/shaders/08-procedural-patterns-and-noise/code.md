# Procedural patterns and noise - typing

This lesson types procedural variation: hash coordinates, interpolate grid
values, build continuous noise, and combine noise at multiple scales.

## Establish the coordinate domain

Start with a centered coordinate that will feed the procedural functions.

```slang id="d8k4pz"
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into normalized coordinates
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;
```

The coordinate now provides a deterministic input for every pixel.

## Hash a coordinate

Create a deterministic irregular value from a two-dimensional coordinate.

```slang id="g5r2vn"
// combine the coordinate components into one scalar
float value = dot(p, float2(127.1, 311.7));

// scramble the scalar through a periodic function
value = sin(value * 43758.5453);

// keep only the fractional portion
value = frac(value);
```

The same coordinate always produces the same value, while nearby lattice
coordinates can produce very different values.

## Separate a cell from its local position

Split a continuous coordinate into its integer cell and fractional position.

```slang id="w7c3mt"
// identify the integer grid cell
float2 cell = floor(p);

// identify the local position inside the cell
float2 local = frac(p);
```

The integer coordinate identifies which grid cell is being sampled, while the
fractional coordinate identifies where the sample lies inside that cell.

## Generate four corner values

Hash the four corners surrounding the current position.

```slang id="r4j8qa"
// generate the value at the lower-left corner
float a = hash(cell);

// generate the value at the lower-right corner
float b = hash(cell + float2(1.0, 0.0));

// generate the value at the upper-left corner
float c = hash(cell + float2(0.0, 1.0));

// generate the value at the upper-right corner
float d = hash(cell + float2(1.0, 1.0));
```

These four values provide the discrete random field that will be interpolated.

## Smooth the local coordinate

Construct interpolation weights that ease into and out of each cell boundary.

```slang id="n2v6yc"
// calculate smooth interpolation weights
float2 u = local * local * (3.0 - 2.0 * local);
```

The weights still range from zero to one, but their transition is smoother
than direct linear interpolation.

## Interpolate the field

Blend the four corners into one continuous value.

```slang id="k8f1dx"
// interpolate the lower pair of corners
float x1 = lerp(a, b, u.x);

// interpolate the upper pair of corners
float x2 = lerp(c, d, u.x);

// interpolate the two horizontal results
float value = lerp(x1, x2, u.y);
```

The result is now a continuous scalar field rather than a grid of hard values.

## Build a reusable noise function

Gather the hashing and interpolation stages into one function.

```slang id="c6q9wr"
// calculate a continuous value field from a coordinate
float noise(float2 p) {
    // identify the integer grid cell
    float2 cell = floor(p);

    // identify the local position inside the cell
    float2 local = frac(p);

    // calculate smooth interpolation weights
    float2 u = local * local * (3.0 - 2.0 * local);

    // generate the lower-left corner value
    float a = hash(cell);

    // generate the lower-right corner value
    float b = hash(cell + float2(1.0, 0.0));

    // generate the upper-left corner value
    float c = hash(cell + float2(0.0, 1.0));

    // generate the upper-right corner value
    float d = hash(cell + float2(1.0, 1.0));

    // interpolate the lower pair of corners
    float x1 = lerp(a, b, u.x);

    // interpolate the upper pair of corners
    float x2 = lerp(c, d, u.x);

    // interpolate the two horizontal results
    return lerp(x1, x2, u.y);
}
```

The function hides the implementation while exposing a simple mathematical
interface: coordinate in, continuous value out.

## Control the noise scale

Use the input coordinate to control the spatial frequency of the field.

```slang id="j3s5vf"
// enlarge the input domain to create smaller features
float value = noise(p * 4.0);

// return the noise field as grayscale
return float4(value, value, value, 1.0);
```

Scaling the input changes the apparent size of the noise without changing the
noise function itself.

## Combine multiple scales

Add several noise fields with increasing frequencies and decreasing amplitudes.

```slang id="t7m4kp"
// start with the broadest noise layer
float value = noise(p * 2.0);

// add a smaller layer with half the influence
value += noise(p * 4.0) * 0.5;

// add a finer layer with one quarter the influence
value += noise(p * 8.0) * 0.25;

// reduce the combined range for display
value *= 0.57;
```

The broad layer establishes the large structure while the finer layers add
detail.

## Threshold the field

Convert the continuous noise into a region mask.

```slang id="u9c2fh"
// evaluate noise at the desired spatial scale
float value = noise(p * 4.0);

// create a hard threshold at the middle of the field
float mask = step(0.5, value);

// return the thresholded field as grayscale
return float4(mask, mask, mask, 1.0);
```

The continuous field now controls which pixels belong to the region.

## Smooth the threshold

Replace the hard transition with a gradual one.

```slang id="a4n7xs"
// evaluate noise at the desired spatial scale
float value = noise(p * 4.0);

// create a soft threshold around the middle of the field
float mask = smoothstep(0.4, 0.6, value);

// return the soft field as grayscale
return float4(mask, mask, mask, 1.0);
```

The same noise field can therefore produce either sharp regions or soft
transitions.

## Use noise for domain warping

Feed the noise result into a coordinate transformation.

```slang id="e6k3qw"
// calculate a low-frequency distortion field
float distortion = noise(p * 3.0);

// copy the original coordinate before warping it
float2 q = p;

// move the domain according to the noise value
q += distortion * 0.15;
```

The noise is no longer the visible pattern. It has become the mathematical
source of the domain deformation.

## Evaluate a pattern after the warp

Use the distorted coordinate with an ordinary repeating function.

```slang id="m8r5yt"
// define the frequency of the final pattern
float frequency = 12.0;

// evaluate the pattern using the warped coordinate
float value = sin(q.x * frequency);

// remap the pattern into a visible range
value = value * 0.5 + 0.5;

// return the warped pattern as grayscale
return float4(value, value, value, 1.0);
```

The pattern remains a sine wave mathematically, but noise has made its domain
irregular.

## Complete procedural field

Assemble the main noise technique into one complete shader-style program.

```slang id="z2q7kc"
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into normalized coordinates
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;

// sample the broadest noise layer
float value = noise(p * 2.0);

// add a medium-frequency noise layer
value += noise(p * 4.0) * 0.5;

// add a fine noise layer
value += noise(p * 8.0) * 0.25;

// normalize the combined value for display
value *= 0.57;

// return the procedural field as grayscale
return float4(value, value, value, 1.0);
```

The final field is generated entirely from the pixel coordinate. No texture
lookup or stored image is required.

## Now type it again

Re-drill the core lattice decomposition and interpolation weights.

```slang id="f5w9qa"
// identify the integer grid cell
float2 cell = floor(p);

// identify the local position inside the cell
float2 local = frac(p);

// calculate smooth interpolation weights
float2 u = local * local * (3.0 - 2.0 * local);
```

Then rebuild the four-corner interpolation.

```slang id="v3k6xm"
// generate the lower-left corner value
float a = hash(cell);

// generate the lower-right corner value
float b = hash(cell + float2(1.0, 0.0));

// generate the upper-left corner value
float c = hash(cell + float2(0.0, 1.0));

// generate the upper-right corner value
float d = hash(cell + float2(1.0, 1.0));

// interpolate the lower pair of corners
float x1 = lerp(a, b, u.x);

// interpolate the upper pair of corners
float x2 = lerp(c, d, u.x);

// interpolate the two horizontal results
float value = lerp(x1, x2, u.y);
```

Finally, drill the multi-scale combination that turns simple noise into a
richer field.

```slang id="q1h8nd"
// start with the broadest noise layer
float value = noise(p * 2.0);

// add a smaller layer with half the influence
value += noise(p * 4.0) * 0.5;

// add a finer layer with one quarter the influence
value += noise(p * 8.0) * 0.25;

// reduce the combined range for display
value *= 0.57;
```

## Wrap up

The flow: coordinate -> hash -> lattice values -> interpolation -> noise field
-> multiple scales -> procedural structure.

Noise is a reusable scalar field whose value can drive patterns, shapes,
distortion, displacement, and color.

