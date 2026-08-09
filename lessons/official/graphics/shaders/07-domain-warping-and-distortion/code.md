# Domain warping and distortion - typing

This lesson types domain distortion: displace coordinates with sine, animate
the displacement, rotate the domain, and evaluate a simple shape afterward.

## Establish the coordinate domain

Start with a centered coordinate that will be transformed before evaluation.

```slang
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into normalized coordinates
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;
```

The coordinate `p` is the original domain and remains unchanged during the
first warp.

## Create a horizontal wave

Use the y coordinate to determine how much x should move.

```slang
// define how quickly the wave repeats
float frequency = 8.0;

// define how far the domain can move
float amplitude = 0.12;

// calculate a vertical wave displacement
float offset = sin(p.y * frequency) * amplitude;

// copy the original domain before modifying it
float2 q = p;

// displace x according to the y coordinate
q.x += offset;
```

The domain is now curved horizontally while the original `p` remains available.

## Turn the warp into a pattern

Evaluate a simple vertical stripe pattern using the warped coordinate.

```slang
// define the number of pattern bands
float bands = 12.0;

// evaluate the pattern in the warped domain
float value = sin(q.x * bands);
```

The pattern is mathematically simple, but its coordinates now follow the wave.

## Convert the pattern into a mask

Map the sine result into a useful visible range.

```slang
// remap the sine range from negative one to one into zero to one
float mask = value * 0.5 + 0.5;

// return the warped pattern as grayscale
return float4(mask, mask, mask, 1.0);
```

The visible distortion came entirely from changing the input domain.

## Animate the displacement

Time can move the phase of the wave continuously.

```slang
// define the animation speed
float speed = 2.0;

// calculate a time-dependent wave displacement
float offset = sin(p.y * frequency + time * speed) * amplitude;

// copy the original domain before modifying it
float2 q = p;

// displace x according to the animated wave
q.x += offset;
```

The geometry does not need to be rebuilt for every frame. The changing time
input changes the coordinate transformation.

## Warp both axes

Use two waves to make the domain deform in both directions.

```slang
// copy the original domain before modifying it
float2 q = p;

// displace x according to the original y coordinate
q.x += sin(p.y * frequency + time) * amplitude;

// displace y according to the original x coordinate
q.y += sin(p.x * frequency + time) * amplitude;
```

Both displacements use `p`, so neither transformation accidentally depends on
the result of the other.

## Rotate the domain

A position-dependent angle can twist the coordinate space.

```slang
// define how strongly the domain twists with height
float twist = 2.0;

// derive a rotation angle from the original y coordinate
float angle = p.y * twist;

// calculate the sine of the rotation angle
float sine = sin(angle);

// calculate the cosine of the rotation angle
float cosine = cos(angle);

// rotate the original x coordinate
float rotatedX = p.x * cosine - p.y * sine;

// rotate the original y coordinate
float rotatedY = p.x * sine + p.y * cosine;

// assemble the rotated domain
float2 q = float2(rotatedX, rotatedY);
```

Because `angle` depends on position, different parts of the domain rotate by
different amounts.

## Evaluate a circle after distortion

A distance equation can remain completely ordinary after the domain is warped.

```slang
// define the radius of the local circle
float radius = 0.3;

// measure the distance from the warped origin
float distance = length(q) - radius;

// convert the distance into a soft boundary mask
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));

// return the distorted circle as grayscale
return float4(shape, shape, shape, 1.0);
```

The circle equation did not change. Only the coordinates supplied to it changed.

## Use polar coordinates

Radial distortion can be expressed by separating distance and angle.

```slang
// calculate the distance from the origin
float radius = length(p);

// calculate the angular position around the origin
float angle = atan2(p.y, p.x);

// define the number of radial waves
float frequency = 16.0;

// define the radial displacement amount
float amplitude = 0.05;

// displace the radial distance with an angular wave
radius += sin(angle * frequency + time) * amplitude;
```

The wave now travels around the origin because its input is angular position
rather than x or y position.

## Reconstruct the warped point

Convert the modified polar coordinates back into Cartesian coordinates.

```slang
// calculate the sine of the modified angle
float sine = sin(angle);

// calculate the cosine of the modified angle
float cosine = cos(angle);

// reconstruct the warped x coordinate
float warpedX = radius * cosine;

// reconstruct the warped y coordinate
float warpedY = radius * sine;

// assemble the warped point
float2 q = float2(warpedX, warpedY);
```

The shape can now use `q` exactly as if it were an ordinary coordinate.

## Complete domain warp

Assemble the main technique into one coherent transformation.

```slang
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into normalized coordinates
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;

// define how quickly the wave repeats
float frequency = 6.0;

// define how far the domain can move
float amplitude = 0.1;

// define the animation speed
float speed = 1.5;

// copy the original domain before modifying it
float2 q = p;

// displace x according to the original y coordinate
q.x += sin(p.y * frequency + time * speed) * amplitude;

// displace y according to the original x coordinate
q.y += sin(p.x * frequency + time * speed) * amplitude;

// define the radius of the local circle
float radius = 0.3;

// measure the distance from the warped origin
float distance = length(q) - radius;

// create a soft transition around the circle boundary
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));

// return the distorted shape as grayscale
return float4(shape, shape, shape, 1.0);
```

The complete flow is original coordinates, coordinate displacement, shape
evaluation, and conversion to the final pixel value.

## Now type it again

Re-drill the core domain-warping operation without re-reading the explanation.

```slang
// define how quickly the wave repeats
float frequency = 8.0;

// define how far the domain can move
float amplitude = 0.12;

// calculate a vertical wave displacement
float offset = sin(p.y * frequency) * amplitude;

// copy the original domain before modifying it
float2 q = p;

// displace x according to the y coordinate
q.x += offset;
```

Now drill the position-dependent rotation.

```slang
// define how strongly the domain twists with height
float twist = 2.0;

// derive a rotation angle from the original y coordinate
float angle = p.y * twist;

// calculate the sine of the rotation angle
float sine = sin(angle);

// calculate the cosine of the rotation angle
float cosine = cos(angle);

// rotate the original x coordinate
float rotatedX = p.x * cosine - p.y * sine;

// rotate the original y coordinate
float rotatedY = p.x * sine + p.y * cosine;

// assemble the rotated domain
float2 q = float2(rotatedX, rotatedY);
```

Finish by drilling the distinction between transforming the domain and
evaluating the shape.

```slang
// define the radius of the local circle
float radius = 0.3;

// measure the distance from the warped origin
float distance = length(q) - radius;

// convert the distance into a soft boundary mask
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));

// return the distorted circle as grayscale
return float4(shape, shape, shape, 1.0);
```

## Wrap up

The flow: coordinate -> displacement or rotation -> warped domain -> shape
evaluation -> pixel value.

The important technique is to deform the input space while keeping the
underlying mathematical model simple.

