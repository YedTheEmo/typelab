# Color ramps and mapping - typing

This lesson types scalar-to-color mapping: normalize a field, shape its
distribution, interpolate palettes, and turn mathematical values into color.

## Establish the coordinate domain

Start with a centered coordinate that will generate the scalar field.

```slang id="r4c7np"
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into normalized coordinates
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;
```

The coordinate is now ready to produce a scalar field.

## Generate a scalar field

Use the coordinate to create a simple periodic field.

```slang id="n6v2qa"
// define the spatial frequency of the field
float frequency = 6.0;

// generate a scalar value from the x coordinate
float value = sin(p.x * frequency);

// remap the sine range into zero to one
value = value * 0.5 + 0.5;
```

The field now has a predictable numerical range suitable for color mapping.

## Remap and clamp

Make the useful portion of a field occupy the visible range.

```slang id="w8k3tf"
// define the lower bound of the useful field
float minimum = 0.25;

// define the upper bound of the useful field
float maximum = 0.75;

// normalize the field into zero to one
float mapped = (value - minimum) / (maximum - minimum);

// prevent the mapped value from leaving the visible range
mapped = clamp(mapped, 0.0, 1.0);
```

The mapping changes the distribution without changing the original field.

## Apply a smooth curve

Use smoothstep to control how the mapped field transitions.

```slang id="q5m9xb"
// soften the transition across the normalized range
float curved = smoothstep(0.0, 1.0, mapped);

// create a grayscale color from the curved value
float3 color = float3(curved);

// return the mapped field
return float4(color, 1.0);
```

The same field can now have a different visual distribution.

## Interpolate between two colors

Use the scalar value as a position along a color ramp.

```slang id="j7r4kc"
// define the color at the low end of the ramp
float3 colorA = float3(0.05, 0.1, 0.3);

// define the color at the high end of the ramp
float3 colorB = float3(1.0, 0.7, 0.15);

// interpolate between the two colors
float3 color = lerp(colorA, colorB, mapped);

// return the color ramp
return float4(color, 1.0);
```

Every scalar value now corresponds to a point between the two endpoint colors.

## Build a three-region ramp

Use two smooth transitions to divide the scalar range into regions.

```slang id="p3v8hd"
// define the low transition
float low = smoothstep(0.0, 0.5, mapped);

// define the high transition
float high = smoothstep(0.5, 1.0, mapped);

// define the low color
float3 lowColor = float3(0.05, 0.1, 0.3);

// define the middle color
float3 middleColor = float3(0.9, 0.4, 0.1);

// define the high color
float3 highColor = float3(1.0, 0.95, 0.5);

// interpolate from low color to middle color
float3 lower = lerp(lowColor, middleColor, low);

// interpolate from middle color to high color
float3 color = lerp(lower, highColor, high);
```

The scalar field now controls movement through three visual regions.

## Reshape the field with a power curve

Change the distribution before applying the palette.

```slang id="z6t2qm"
// push intermediate values toward the lower end
float curved = pow(mapped, 2.0);

// use the curved value as the palette position
float3 color = lerp(colorA, colorB, curved);

// return the curved palette
return float4(color, 1.0);
```

The palette has not changed. The values entering it have changed.

## Build a procedural palette

Generate RGB channels mathematically from one scalar.

```slang id="c9w5ra"
// define a phase offset for each color channel
float3 phase = float3(0.0, 0.33, 0.67);

// convert the scalar into a continuous color cycle
float3 color = 0.5 + 0.5 * cos(6.28318 * (mapped + phase));

// return the procedural palette
return float4(color, 1.0);
```

The palette itself is now a mathematical function.

## Make the palette reusable

Separate palette generation from the rest of the shader.

```slang id="f2k7mx"
// convert a normalized scalar into a procedural color
float3 palette(float t) {
    // define a phase offset for each color channel
    float3 phase = float3(0.0, 0.33, 0.67);

    // generate the three color channels
    return 0.5 + 0.5 * cos(6.28318 * (t + phase));
}
```

The shader can now generate a field and pass its result into `palette`.

## Map a distance field

Use a circle distance to demonstrate mapping a geometric field.

```slang id="v5n8qd"
// define the circle radius
float radius = 0.3;

// calculate the signed distance to the circle
float distance = length(p) - radius;

// convert the inside region into a soft mask
float inside = smoothstep(0.02, -0.02, distance);

// define the interior color
float3 color = float3(0.1, 0.35, 0.8);

// mix the interior with the background
color *= inside;

// return the mapped distance field
return float4(color, 1.0);
```

The signed distance becomes visual information through the smooth threshold.

## Combine field and palette

Use a noise-like field and send it through a reusable palette.

```slang id="m1r6yc"
// generate a scalar field from the coordinate
float value = noise(p * 3.0);

// reshape the field distribution
float mapped = smoothstep(0.2, 0.8, value);

// convert the scalar field into a color
float3 color = palette(mapped);

// return the colored field
return float4(color, 1.0);
```

The field, mapping, and palette remain separate stages.

## Complete color mapping

Assemble the full scalar-to-color pipeline.

```slang id="k8q4zw"
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into normalized coordinates
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;

// generate a procedural scalar field
float value = noise(p * 3.0);

// normalize the useful range
float mapped = clamp((value - 0.2) / 0.6, 0.0, 1.0);

// reshape the scalar distribution
mapped = smoothstep(0.0, 1.0, mapped);

// define the phase offset for the palette
float3 phase = float3(0.0, 0.33, 0.67);

// generate a continuous color palette
float3 color = 0.5 + 0.5 * cos(6.28318 * (mapped + phase));

// return the final mapped color
return float4(color, 1.0);
```

The scalar field is generated first, then normalized, curved, and finally
converted into RGB.

## Now type it again

Re-drill normalization and clamping.

```slang id="u4p7ne"
// define the lower bound of the useful field
float minimum = 0.25;

// define the upper bound of the useful field
float maximum = 0.75;

// normalize the field into zero to one
float mapped = (value - minimum) / (maximum - minimum);

// prevent the mapped value from leaving the visible range
mapped = clamp(mapped, 0.0, 1.0);
```

Then drill a two-color ramp.

```slang id="b6t9xm"
// define the color at the low end of the ramp
float3 colorA = float3(0.05, 0.1, 0.3);

// define the color at the high end of the ramp
float3 colorB = float3(1.0, 0.7, 0.15);

// interpolate between the two colors
float3 color = lerp(colorA, colorB, mapped);
```

Finish with the scalar-to-palette pipeline.

```slang id="e3q8vr"
// generate a procedural scalar field
float value = noise(p * 3.0);

// normalize the useful range
float mapped = clamp((value - 0.2) / 0.6, 0.0, 1.0);

// reshape the scalar distribution
mapped = smoothstep(0.0, 1.0, mapped);

// define the phase offset for the palette
float3 phase = float3(0.0, 0.33, 0.67);

// generate a continuous color palette
float3 color = 0.5 + 0.5 * cos(6.28318 * (mapped + phase));

// return the final mapped color
return float4(color, 1.0);
```

## Wrap up

The flow: field -> normalize -> reshape -> palette -> color.

The field contains the information; mapping determines how strongly that
information appears, and the palette determines how numerical values become
visual color.

