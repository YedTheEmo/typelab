````markdown
# Shape blending and morphing - typing

This lesson types boolean SDF operations, smooth unions, linear
interpolation, and simple field morphing.

## Combine two shapes with a union

The minimum of two SDFs produces their basic union.

```slang
// store the point being evaluated
float2 point = float2(0.2, 0.0);

// calculate the first circle's signed distance
float first = length(point - float2(-0.4, 0.0)) - 0.5;

// calculate the second circle's signed distance
float second = length(point - float2(0.4, 0.0)) - 0.5;

// keep the field belonging to the closer shape
float unionDistance = min(first, second);
```

The important operation is:

```slang
float unionDistance = min(first, second);
```

The two shapes remain hard-edged where their fields meet.

## Intersect two shapes

The maximum produces the basic intersection.

```slang
// store the point being evaluated
float2 point = float2(0.2, 0.0);

// calculate the first circle's signed distance
float first = length(point - float2(-0.2, 0.0)) - 0.6;

// calculate the second circle's signed distance
float second = length(point - float2(0.2, 0.0)) - 0.6;

// keep the field required by both shapes
float intersectionDistance = max(first, second);
```

The important operation is:

```slang
float intersectionDistance = max(first, second);
```

A point remains inside the result only when it satisfies both fields.

## Subtract one shape from another

Negating one field reverses its inside/outside relationship.

```slang
// store the point being evaluated
float2 point = float2(0.0, 0.0);

// calculate the distance to the shape being kept
float base = length(point) - 0.8;

// calculate the distance to the shape being removed
float cutter = length(point - float2(0.3, 0.0)) - 0.4;

// reverse the cutter's inside and outside relationship
float invertedCutter = -cutter;

// keep the base while removing the cutter
float subtractionDistance = max(base, invertedCutter);
```

The structure is:

```text
A - B = max(A, -B)
```

## Smoothly join two fields

A smooth minimum replaces the hard transition of `min`.

One common polynomial form is:

```slang
// store the point being evaluated
float2 point = float2(0.0, 0.0);

// calculate the first circle field
float first = length(point - float2(-0.4, 0.0)) - 0.5;

// calculate the second circle field
float second = length(point - float2(0.4, 0.0)) - 0.5;

// control the width of the smooth transition
float blend = 0.4;

// find the signed difference between the fields
float h = saturate(0.5 + 0.5 * (second - first) / blend);

// interpolate between the fields
float mixed = lerp(second, first, h);

// subtract the smoothing correction
float smoothUnion = mixed - blend * h * (1.0 - h);
```

The important thing to understand is not the formula as a memorized recipe.

The formula creates a region where the two fields transition continuously
instead of abruptly switching from one to the other.

## Interpolate between two shapes

Distance fields are scalar values, so they can be interpolated.

```slang
// store the point being evaluated
float2 point = float2(0.3, 0.1);

// calculate a circle field
float circle = length(point) - 0.6;

// calculate a box field
float2 halfSize = float2(0.45, 0.35);
float2 q = abs(point) - halfSize;
float outside = length(max(q, 0.0));
float inside = min(max(q.x, q.y), 0.0);
float box = outside + inside;

// control the position in the morph
float t = 0.5;

// interpolate between the two fields
float morph = lerp(circle, box, t);
```

At:

```slang
t = 0.0;
```

the result is the circle field.

At:

```slang
t = 1.0;
```

the result is the box field.

Intermediate values produce intermediate scalar fields.

## Shape the morph parameter

The interpolation parameter does not have to change linearly.

```slang
// store the raw morph progress
float t = 0.5;

// remap the progress into a smooth transition
float smoothT = smoothstep(0.0, 1.0, t);

// interpolate using the shaped parameter
float morph = lerp(circle, box, smoothT);
```

The field interpolation and the timing curve are separate concepts.

## Now type it again

Type the central construction pattern from a clean start.

```slang
// store the point being evaluated
float2 point = float2(0.2, 0.0);

// calculate the first primitive field
float first = length(point - float2(-0.4, 0.0)) - 0.5;

// calculate the second primitive field
float second = length(point - float2(0.4, 0.0)) - 0.5;

// create a hard union
float hardUnion = min(first, second);

// control the smooth transition width
float blend = 0.3;

// calculate the normalized field difference
float h = saturate(0.5 + 0.5 * (second - first) / blend);

// blend the two fields
float mixed = lerp(second, first, h);

// create the smooth union
float smoothUnion = mixed - blend * h * (1.0 - h);

// control the morph between the fields
float t = 0.5;

// interpolate the two descriptions
float morph = lerp(first, second, t);
```

The essential pattern is:

```text
primitive A ──┐
              ├─> field operation -> resulting field
primitive B ──┘
```

The final geometry is determined by the resulting field's zero surface.

## Wrap up

SDFs turn geometry into scalar fields, and scalar operations let those fields
be unioned, intersected, subtracted, smoothly blended, or interpolated into
new mathematical forms.
```
````

