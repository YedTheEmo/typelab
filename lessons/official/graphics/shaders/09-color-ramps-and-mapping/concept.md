# Color ramps and mapping - concepts

A shader often produces a scalar value before it produces a color. A distance
field produces a distance. Noise produces a value. A lighting calculation
produces an intensity. None of these values are colors by themselves.

Color mapping is the process of turning that scalar field into a visual range.
The important idea is that the underlying field and its appearance are
separate mathematical stages.

A useful shader therefore does not immediately decide that a value should be
red, green, or blue. It first understands the numerical range of the field,
then chooses a mapping that makes the important parts of that range visible.

## Scalar fields are numerical information

Suppose a shader has a value that ranges from zero to one.

```slang
float value = noise(p);
```

The value itself contains spatial information. Nearby pixels may have similar
values, and different regions may contain different values.

Returning that value in every color channel produces grayscale:

```slang
float3 color = float3(value);
```

This is useful because it exposes the field directly. Bright pixels represent
larger values and dark pixels represent smaller values.

Color mapping changes this final interpretation without changing the field
itself.

## Remapping a range

Not every function naturally produces values in the range zero to one. A
distance might range from negative values to large positive values, while a
sine function ranges from negative one to one.

A linear remapping converts one interval into another.

```slang
float mapped = (value - minimum) / (maximum - minimum);
```

Subtracting `minimum` moves the lower bound to zero. Dividing by the range
compresses the result so that the upper bound becomes one.

For example, a value between 2 and 6 can be normalized into a value between
zero and one.

This operation is fundamental because later mapping functions usually become
easier to reason about when their input has a known range.

## Clamp the result

A remapping does not automatically prevent values outside the expected range.

If the original value is below the minimum, the normalized result becomes
negative. If it is above the maximum, the result becomes greater than one.

A clamp restricts the result:

```slang
float mapped = clamp(value, 0.0, 1.0);
```

Values below zero become zero and values above one become one.

Clamping is useful when a mapping represents a bounded visual quantity such as
color intensity, opacity, or a mask.

It is important to understand that clamping discards information outside the
range. That can be desirable for presentation, but it should not be confused
with preserving the original field.

## Linear interpolation between colors

Once a scalar has been normalized, it can control a transition between colors.

```slang
float3 color = lerp(colorA, colorB, value);
```

When `value` is zero, the result is `colorA`. When it is one, the result is
`colorB`. Values between them produce a continuous interpolation.

This creates a color ramp with two endpoints.

The scalar field determines where each point lies along the ramp. The colors
only define how that numerical position should appear.

This separation makes the same field reusable. One ramp can make it look like
fire, another like ice, and another like a grayscale height map without
changing the field-generation mathematics.

## A three-color ramp

Two colors are sometimes insufficient. A field may need one color for low
values, another around the middle, and a third for high values.

One approach is to split the normalized range at a midpoint.

```slang
float lower = smoothstep(0.0, 0.5, value);
float upper = smoothstep(0.5, 1.0, value);
```

The first transition controls the lower half and the second controls the
upper half.

The two resulting weights can then be used to construct a piecewise palette.
The important idea is not the specific colors. It is that a scalar interval
can be divided into meaningful regions.

## Smoothstep is a mapping curve

A color ramp does not have to change at a constant rate.

The `smoothstep` function maps an input into a smooth transition between two
edges:

```slang
float mask = smoothstep(edge0, edge1, value);
```

Below `edge0`, the result is zero. Above `edge1`, it is one. Between them, the
result follows a smooth curve.

This makes `smoothstep` useful for more than anti-aliasing. It is a general
way of converting a scalar field into a controlled transition.

A distance field can use it to create a border. Noise can use it to isolate
regions. Lighting can use it to soften a threshold.

Color mapping uses the same mathematical mechanism.

## Changing the curve

Linear interpolation and smoothstep produce different distributions of values.

A linear mapping preserves a constant rate of change:

```slang
float mapped = value;
```

A smooth mapping eases near its endpoints:

```slang
float mapped = smoothstep(0.0, 1.0, value);
```

This matters because the distribution of the scalar field determines how much
of the image receives each color.

A mapping curve can therefore emphasize certain regions without changing the
underlying data.

The shader is not merely choosing a color. It is choosing how numerical
differences become visual differences.

## Power curves

A power function provides another simple way to reshape a normalized value.

```slang
float mapped = pow(value, 2.0);
```

For values between zero and one, squaring pushes intermediate values toward
zero. A power below one does the opposite and expands the higher part of the
range.

This is useful when the raw field contains useful information but its
distribution does not produce enough visual contrast.

For example, a dark field can be brightened by using a fractional exponent:

```slang
float mapped = pow(value, 0.5);
```

The important principle is that the field remains unchanged. Only its mapping
into the visible range changes.

## Palette functions

A more flexible palette can be constructed directly from a mathematical
function.

One useful family uses phase-shifted cosine waves:

```slang
float3 color = 0.5 + 0.5 * cos(6.28318 * (value + phase));
```

The cosine is evaluated independently for the three color channels through
different phase values.

This produces a continuous path through RGB space as `value` changes.

The exact constants are less important than the structure. A scalar value is
used as the input to a periodic function, and the resulting channels become
the color.

This means a palette itself can be procedural.

## Designing a palette as a function

A palette can be expressed as a function from one scalar to one color.

```slang
float3 palette(float t) {
    return 0.5 + 0.5 * cos(6.28318 * (t + phase));
}
```

Now the rest of the shader does not need to know how the colors are produced.

It only needs to provide a normalized value:

```slang
float3 color = palette(value);
```

This is another example of compositional shader design. The field generator
produces data, while the palette function decides how that data appears.

The same palette function can therefore be reused with noise, distance,
lighting, or animation.

## Mapping a distance field

Distance fields are particularly useful for demonstrating color mapping
because the raw value has a clear geometric meaning.

For a circle:

```slang
float distance = length(p) - radius;
```

The zero crossing identifies the surface. Negative values are inside the
circle and positive values are outside.

A color mapping can turn that information into an interior, border, and
background:

```slang
float inside = smoothstep(0.02, -0.02, distance);
```

The sign of the distance now becomes part of the visual classification.

The important point is that the geometry did not change. The mapping merely
interpreted a numerical property of the geometry.

## Multiple thresholds

A scalar field can be classified into several regions by applying multiple
transitions.

```slang
float low = smoothstep(0.2, 0.3, value);
float high = smoothstep(0.7, 0.8, value);
```

The low transition identifies where the field leaves its lower region, while
the high transition identifies where it enters its upper region.

These masks can be combined with colors to create a controlled ramp.

This technique is useful for terrain-like fields, heat maps, stylized
lighting, and any effect where different numerical ranges have different
visual meanings.

## Contrast is a mapping operation

Suppose a scalar field mostly occupies the range from 0.4 to 0.6. Mapping
zero to one linearly wastes most of the available visual range.

Instead, remap the useful interval:

```slang
float contrast = smoothstep(0.4, 0.6, value);
```

Values below the useful interval become dark, values above it become bright,
and the narrow region between them receives most of the transition.

This is effectively contrast control expressed as a mathematical function.

The lesson is important beyond color. Whenever a shader result looks flat,
ask whether the underlying field lacks variation or whether the mapping is
failing to expose the variation that already exists.

## Color is the final interpretation

A useful procedural shader can therefore be understood as several separate
functions.

```slang
float value = field(p);
float mapped = mapping(value);
float3 color = palette(mapped);
```

The first stage creates information. The second reshapes its numerical
distribution. The third turns the result into a color.

Keeping these stages separate makes shader experimentation much easier. A
change to the palette should not require changing the noise function. A change
to the contrast should not require changing the geometry.

The visual result is a composition of independent mathematical mappings.

## The deeper mental model

A shader field is data, not appearance.

Noise produces variation. Distance produces geometry. Lighting produces an
intensity. Color mapping interprets those values and determines how strongly
different numerical regions are represented visually.

The most important habit is therefore to inspect the scalar field before
complicating its source. If the field contains useful structure, a different
remapping or palette may reveal it immediately.

The general process is:

```slang
float value = field(p);
float mapped = mapping(value);
float3 color = palette(mapped);
```

Once this separation becomes intuitive, color stops being a collection of
arbitrary RGB values and becomes another mathematical transformation in the
shader pipeline.

## Next step

Now type the code version of this lesson.

