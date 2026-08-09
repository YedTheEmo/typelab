````markdown
# Vector intuition and trig - concepts

A shader becomes much more expressive once coordinates stop being treated as
just pairs or triples of numbers. A coordinate is a point in space, but the
same numbers can also describe a direction, an offset, or a velocity. The
difference is not the data type itself. The meaning comes from how the
numbers are used.

Vectors provide the mathematical language for these relationships. They let
a shader describe where something is, which direction something points, how
far apart two things are, and how strongly two directions agree.

Trigonometric functions add another important capability. Sine and cosine let
a shader turn an angle into a position on a circle, or turn a changing value
into smooth periodic motion. Together, vectors and trigonometry form much of
the basic vocabulary from which procedural visual effects are constructed.

## A vector describes a relationship

A 2D vector contains two components:

```
float2 v = float2(3.0, 2.0);
```

Those numbers can represent a position, but they can also represent an
offset from one position to another.

If a point is at `(4, 3)` and another point is at `(7, 5)`, their difference is:

```
float2 direction = float2(7.0, 5.0) - float2(4.0, 3.0);
```

The result is `(3, 2)`.

That does not describe the second point itself. It describes how to travel
from the first point to the second: three units horizontally and two units
vertically.

This distinction becomes essential in shader mathematics. We constantly
turn positions into vectors describing relationships between positions.

If `p` is the current location and `target` is another location:

```
float2 toTarget = target - p;
```

The vector points from the current location toward the target.

## Vector length is distance

A vector also contains a magnitude, or length.

For a 2D vector `(x, y)`, its length comes from the same geometry used by a
right triangle:

```
length = sqrt(x * x + y * y)
```

Shader languages provide this operation directly:

```
float distance = length(toTarget);
```

If `toTarget` describes the displacement from one point to another, its length
is the distance between those points.

This gives us an extremely useful conversion:

```
position -> difference vector -> length -> distance
```

A shader can therefore turn spatial relationships into scalar values.

Suppose the current point is `(3, 4)` relative to some center:

```
float2 offset = float2(3.0, 4.0);
float distance = length(offset);
```

The distance is `5`.

The vector retains directional information. The length discards the direction
and keeps only how far away the point is.

That distinction appears constantly in procedural graphics. Sometimes we need
to know where something is relative to another object. Sometimes we only need
to know how far away it is.

## Normalization keeps direction and removes magnitude

Consider two vectors:

```
float2 a = float2(2.0, 0.0);
float2 b = float2(20.0, 0.0);
```

They point in the same direction, but their lengths are different.

If we only care about direction, those different magnitudes are unnecessary.
Normalization converts a nonzero vector into a vector whose length is one.

```
float2 direction = normalize(a);
```

The resulting vector points the same way as `a`, but its magnitude is one.

Conceptually:

```
vector = direction * magnitude
```

Normalization throws away the magnitude and keeps the direction.

This is particularly important for lighting and geometry. A direction toward a
light should normally describe where the light is, not accidentally become
stronger merely because the vector happens to have a larger magnitude.

Normalization lets us say:

"I care about which way this points."

rather than:

"I care about which way this points and how long the vector happened to be."

## Positions and directions behave differently

A useful mental distinction is that a position answers:

"Where is it?"

A direction answers:

"Which way is it?"

A position such as:

```
float3 position = float3(4.0, 2.0, 1.0);
```

identifies a location in space.

A direction such as:

```
float3 direction = float3(0.0, 1.0, 0.0);
```

describes movement toward positive Y.

The same vector type can store both. The mathematics performed on it determines
the interpretation.

Subtracting two positions gives a direction:

```
float3 direction = target - position;
```

Adding a direction to a position produces another position:

```
float3 nextPosition = position + direction;
```

This is one of the most important relationships in graphics.

A large amount of geometry can be understood through variations of:

```
position + direction * distance
```

which means "start here, travel in this direction, for this distance."

Raymarching later in the track will rely heavily on exactly this idea.

## The dot product compares directions

The dot product combines two vectors into one scalar.

```
float value = dot(a, b);
```

For normalized vectors, this scalar has a particularly useful geometric
meaning: it describes the angle between the directions.

If two normalized vectors point in exactly the same direction:

```
dot(a, b) = 1
```

If they are perpendicular:

```
dot(a, b) = 0
```

If they point in opposite directions:

```
dot(a, b) = -1
```

So the dot product can act as a measure of directional agreement.

Imagine standing on a surface and looking toward a light. Let `normal` describe
which way the surface faces and `lightDirection` describe which way the light
is located.

```
float brightness = dot(normal, lightDirection);
```

If the surface faces the light directly, the value is high.

If the surface is turned sideways, the value approaches zero.

If the surface faces away from the light, the value becomes negative.

For basic diffuse lighting, negative values are usually removed:

```
float brightness = max(dot(normal, lightDirection), 0.0);
```

The important insight is not the lighting formula itself. The insight is that
a visual property such as "how directly does this surface face the light?"
can be represented by comparing two directions.

## Dot products also project one vector onto another

The dot product has another useful interpretation.

Suppose `a` is some vector and `b` is a normalized direction. Then:

```
dot(a, b)
```

tells us how much of `a` lies along `b`.

If `a` points strongly in the same direction as `b`, the result is large.

If `a` is perpendicular to `b`, the result is zero.

If `a` points against `b`, the result is negative.

This makes the dot product useful far beyond lighting. It can measure alignment,
extract directional components, and help determine whether a point or surface
faces a particular direction.

Shader mathematics repeatedly asks questions about direction, so an operation
that converts two directions into one meaningful scalar becomes extremely
valuable.

## Sine describes smooth periodic change

Sine is often introduced as a function to memorize, but shader programming
benefits more from understanding its shape.

As its input changes continuously, sine smoothly oscillates between `-1` and
`1`.

```
float value = sin(x);
```

If `x` increases steadily, `value` repeatedly rises and falls.

The important property is continuity. There are no sudden jumps between values.
This makes sine useful whenever something should vary smoothly and repeatedly.

For example, a value that oscillates over time can be created with:

```
float wave = sin(time);
```

The value moves through a repeating cycle.

The same principle works spatially. If the input is a coordinate:

```
float wave = sin(position.x);
```

the result varies across space instead of time.

This turns a one-dimensional coordinate into a repeating pattern.

## Frequency controls how quickly the pattern repeats

We can multiply the input to sine:

```
float wave = sin(position.x * frequency);
```

Increasing `frequency` makes the oscillations happen more rapidly as the
coordinate changes.

This is easier to understand visually than as a purely algebraic operation.

The coordinate is moving through the input domain of sine. Multiplying it by a
larger number means that the coordinate travels through more cycles over the
same physical distance.

Therefore:

```
sin(x)
```

might produce a broad wave, while:

```
sin(x * 10.0)
```

produces many smaller waves across the same region.

This becomes useful for stripes, waves, ripples, distortion, and many forms
of procedural pattern generation.

## Amplitude controls how strongly the value changes

Frequency controls how often a wave repeats. Amplitude controls how far its
output reaches.

The basic sine function already produces values from `-1` to `1`:

```
float wave = sin(x);
```

Multiplying the result changes its range:

```
float wave = sin(x) * amplitude;
```

With an amplitude of `2`, the result ranges from `-2` to `2`.

The distinction is important:

```
sin(x * frequency) * amplitude
```

changes two independent properties.

The input multiplier changes the spatial or temporal frequency.

The output multiplier changes the magnitude of the resulting variation.

When building procedural effects, separating these concepts makes it much
easier to reason about what each parameter is doing.

## Cosine is the same cycle with a different phase

Cosine behaves like sine but begins at a different point in its cycle.

```
float x = cos(angle);
float y = sin(angle);
```

This pairing is one of the most important uses of trigonometry in graphics.

For a given angle, cosine provides the horizontal component and sine provides
the vertical component of a point on a unit circle.

Together they produce:

```
float2 point = float2(cos(angle), sin(angle));
```

As `angle` changes, `point` travels around the circle.

This is not merely a trick for drawing circles. It gives us a way to convert an
angular description into a 2D direction.

That means an angle can become a vector.

## Angles can become directions

Suppose an angle is represented by `angle`.

We can construct a unit direction:

```
float2 direction = float2(cos(angle), sin(angle));
```

Because sine and cosine satisfy the geometry of the unit circle, the resulting
vector has length one.

We can then scale it:

```
float2 offset = direction * radius;
```

Now the vector points in the direction described by the angle and reaches a
distance described by the radius.

A point on a circle centered at `center` can therefore be constructed as:

```
float2 point = center + direction * radius;
```

This gives us a direct bridge between trigonometry and geometry.

Angles describe orientation.

Sine and cosine convert orientation into components.

Vectors represent those components.

Scalar multiplication controls distance.

Vector addition moves the result into the desired location.

## Time and space use the same mathematics

A powerful shader idea is that the same function can vary across either space
or time.

If we write:

```
float value = sin(x);
```

the value changes when `x` changes.

If `x` is a spatial coordinate, we get a spatial pattern.

If `x` is time, we get animation.

If `x` combines both:

```
float value = sin(x + time);
```

the pattern changes across space and shifts as time changes.

This means many animated effects are not fundamentally separate from static
procedural patterns. Animation can simply be another input to the same
mathematical function.

The visual effect emerges because the function is being evaluated with a
changing input.

## Mapping values into useful ranges

Many mathematical operations produce values that are inconvenient for direct
display.

Sine produces values from `-1` to `1`, while color channels are commonly
represented in a nonnegative range.

A simple transformation is:

```
float value = sin(x) * 0.5 + 0.5;
```

The multiplication reduces the range from `[-1, 1]` to `[-0.5, 0.5]`.

The addition shifts it to `[0, 1]`.

The result is therefore a smooth repeating value suitable for many normalized
uses.

This pattern appears constantly in shaders:

```
[-1, 1] -> [0, 1]
```

It is not a special shader feature. It is simply a change of range.

Understanding these transformations is more useful than memorizing that
particular expression.

## Vectors can be transformed component by component

Shader vector operations are often designed to operate naturally on every
component.

For example:

```
float2 a = float2(2.0, 3.0);
float2 b = float2(4.0, 5.0);
float2 result = a + b;
```

produces:

```
(6, 8)
```

The addition happens component by component.

Multiplying a vector by a scalar:

```
float2 result = a * 2.0;
```

scales both components.

This is useful because the same operation can be applied to an entire
coordinate without separately handling X and Y.

A shader therefore often reads like compact mathematics because vector
operations allow a whole geometric quantity to be manipulated at once.

## The important operations form a small vocabulary

At this stage, the most useful mental vocabulary is:

```
subtract positions -> get a displacement
length -> get a distance
normalize -> keep direction, discard magnitude
dot -> compare or project directions
sin -> smooth periodic variation
cos -> smooth periodic variation and circular coordinates
multiply input -> change frequency
multiply output -> change amplitude
```

These are not isolated tricks.

They are transformations that can be chained together.

For example:

```
position
    -> subtract center
    -> length
    -> scale
    -> remap
```

can produce a radial gradient.

Or:

```
position.x
    -> multiply frequency
    -> sin
    -> remap
```

can produce a repeating stripe pattern.

Or:

```
angle
    -> sin and cos
    -> vector
    -> multiply radius
    -> add center
```

can generate points moving around a circle.

The power comes from composition.

## The mathematical mindset

When looking at a visual effect, avoid immediately asking which shader trick
produces it.

Instead ask what quantity changes across the image.

Does it depend on distance from a point?

Does it depend on direction?

Does it repeat periodically?

Does it need a normalized direction?

Does it depend on how closely two directions align?

Once that question has been answered, the appropriate mathematical operation
often becomes much easier to identify.

A bright spot suggests a value that depends on distance.

A directional response suggests a dot product.

A repeating wave suggests sine or cosine.

A circular arrangement suggests converting an angle into a vector.

An effect that should be independent of vector magnitude suggests
normalization.

This is the beginning of thinking in shader mathematics rather than shader
recipes.

## Next step

Now type the code version of this lesson.
````

