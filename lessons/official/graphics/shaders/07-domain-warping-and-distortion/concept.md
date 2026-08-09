# Domain warping and distortion - concepts

Repetition and folding changed a coordinate by moving it into a different
region of space. Domain warping goes one step further: instead of mapping a
point to a repeated or mirrored location, it continuously moves the point
according to a mathematical function.

The important idea is that the visual function does not need to know that the
space was warped. A circle can still be defined by distance from the origin,
but the coordinates given to that circle can move differently at every pixel.
The resulting image looks curved, stretched, rippled, twisted, or organic
even though the underlying shape equation has not changed.

## Move the input instead of the shape

A shape can be thought of as a function that accepts a coordinate and returns
some value.

```slang
float distance = length(p) - radius;
```

This equation asks how far the point `p` is from a circle of the given radius.
If `p` changes, the circle's mathematical result changes even though the
equation itself stays exactly the same.

Domain warping inserts another function between the original coordinate and
the shape:

```slang
float2 q = warp(p);
float distance = length(q) - radius;
```

The circle is still evaluated normally. The interesting behavior comes from
the fact that `q` is no longer the original coordinate.

This separation is fundamental to procedural graphics. One function can define
a shape, while another independently defines how space is distorted around
that shape.

## A sine wave as a displacement

The simplest useful warp is a periodic displacement. The sine function
produces a smooth value that repeatedly moves between -1 and 1.

```slang
float offset = sin(p.y * frequency) * amplitude;
```

The y coordinate determines where we are along the wave. Multiplying by
`frequency` controls how quickly the wave oscillates, while `amplitude`
controls how far the coordinate is displaced.

The result is not itself a distorted shape. It is a displacement amount.
Apply it to another coordinate:

```slang
float2 q = p;
q.x += offset;
```

Now points at different heights receive different horizontal offsets. A
vertical structure evaluated using `q` will appear to wave from side to side.

The important distinction is between changing a value and changing the domain.
The sine function does not bend the shape directly. It changes which point of
the shape's mathematical domain is being sampled.

## Frequency and amplitude

Frequency and amplitude control two different properties of a deformation.

A low frequency produces a broad, slow curve across the domain. A high
frequency produces many tightly packed oscillations.

```slang
float offset = sin(p.y * 3.0) * 0.2;
```

The multiplier inside `sin` controls frequency. The multiplier outside it
controls displacement magnitude.

This distinction appears throughout procedural graphics. The same mathematical
function can produce dramatically different structures simply by changing the
scale of its input and output.

A useful mental model is that the input scale controls how quickly the pattern
changes through space, while the output scale controls how strongly that
change moves the domain.

## Warping both coordinates

A two-dimensional domain can be warped in both directions.

```slang
float2 q = p;
q.x += sin(p.y * frequency) * amplitude;
q.y += sin(p.x * frequency) * amplitude;
```

The x coordinate is displaced according to y, while y is displaced according
to x. This creates a coupled deformation rather than two independent waves.

The order matters. If the second expression used the already modified `q.x`
instead of the original `p.x`, the second displacement would depend on the
first displacement. That produces a different mathematical transformation.

Keeping the original coordinate separate from the warped coordinate makes the
relationship explicit:

```slang
float2 q = p;
q.x += sin(p.y * frequency) * amplitude;
q.y += sin(p.x * frequency) * amplitude;
```

Here `p` describes the original domain and `q` describes its transformed
version.

## Time turns a warp into motion

A spatial function becomes an animation when time changes its phase.

```slang
float offset = sin(p.y * frequency + time) * amplitude;
```

The same point now receives a different displacement as time advances.

The variable `time` is not special because it is a shader concept. It is simply
another input to the mathematical function. As its value changes, the function
produces a different domain transformation.

Increasing time continuously moves the phase of the sine wave. The result is
continuous motion because neighboring time values produce neighboring
displacements.

This is the same principle as spatial variation. A coordinate describes where
we are in space, while time describes where we are in the animation.

## Rotation as domain distortion

Warping does not have to mean adding an offset. A coordinate can also be
rotated by an angle that depends on its position.

A two-dimensional rotation has the form:

```slang
float2 q;
q.x = p.x * cos(angle) - p.y * sin(angle);
q.y = p.x * sin(angle) + p.y * cos(angle);
```

If `angle` is constant, this is simply a rotation of the entire domain.

If the angle varies according to position, different regions of the domain
rotate by different amounts:

```slang
float angle = p.y * twist;
```

Substituting that angle into the rotation means the top and bottom of the
domain can rotate differently. A shape evaluated afterward therefore appears
twisted.

The shape has not been rotated independently at each point. The coordinate
system used to evaluate it has been rotated differently at each point.

## Polar coordinates expose radial structure

Some distortions become easier to understand when a point is described by
radius and angle instead of x and y.

The radius from the origin is:

```slang
float radius = length(p);
```

The angular position can be obtained with:

```slang
float angle = atan2(p.y, p.x);
```

Now a radial pattern can be manipulated through `angle` while its distance from
the origin remains separate.

For example, adding a sine wave to the radius creates radial waves:

```slang
float radius = length(p);
radius += sin(angle * frequency) * amplitude;
```

The deformation now repeats around the circle rather than across a straight
Cartesian axis.

This demonstrates a broader principle: choosing a useful coordinate system
can make a particular visual transformation much easier to express.

## Warping a coordinate repeatedly

One warp can become the input to another warp.

```slang
float2 q = p;
q.x += sin(q.y * frequency) * amplitude;
q.y += sin(q.x * frequency) * amplitude;
```

The second transformation sees the result of the first. The domain therefore
becomes progressively more distorted.

Repeated transformations are powerful, but they also make the relationship
between input and output harder to reason about. A useful shader usually starts
with one simple deformation and adds complexity only when the visual result
requires it.

The goal is not to make the formula complicated. The goal is to construct a
mathematical transformation whose behavior is understandable.

## Nested warping

A common procedural technique is to use one warped coordinate to control
another mathematical function.

```slang
float2 q = p;
q.x += sin(q.y * frequency) * amplitude;

float value = sin(q.x * bands);
```

The first sine changes the domain. The second sine evaluates a repeating
pattern inside that altered domain.

Without the warp, the second function creates regular bands. With the warp,
those bands bend because the coordinates entering the pattern have been
displaced.

This is one of the most important ideas in procedural visual effects:
complex-looking structure can emerge by composing simple functions rather
than by writing a complicated shape equation.

## Preserve the distinction between domain and value

A shader can manipulate the value returned by a function, or it can manipulate
the coordinates given to the function. These operations have different
meanings.

Consider a simple pattern:

```slang
float value = sin(p.x * bands);
```

Changing `value` afterward changes the appearance of the already evaluated
pattern. Changing `p` before the evaluation changes where the pattern exists
in space.

Domain warping therefore belongs before the shape or pattern evaluation:

```slang
float2 q = warp(p);
float value = pattern(q);
```

This ordering lets one domain transformation affect many different functions.
The same warped coordinate could be used for a distance field, a stripe
pattern, a color lookup, or a lighting calculation.

## Combining warps with previous techniques

Domain warping is especially useful when combined with repetition and folding.

A point can first be repeated into a local cell:

```slang
float2 q = p - cellSize * floor(p / cellSize + 0.5);
```

The local coordinate can then be warped:

```slang
q.x += sin(q.y * frequency) * amplitude;
```

Finally, the warped coordinate can be evaluated by a familiar shape:

```slang
float distance = length(q) - radius;
```

The three operations have separate responsibilities. Repetition determines
which local copy is being evaluated, warping changes the shape of that local
domain, and the distance equation determines what the local object is.

This composability is what makes domain manipulation such an important part of
shader mathematics.

## The deeper mental model

A procedural shader can be viewed as a chain of functions.

The original pixel coordinate enters the chain. Each transformation changes
the coordinate or the value derived from it. The final mathematical function
turns the transformed coordinate into a scalar or color.

```slang
float2 q = warp(repeat(p));
float value = shape(q);
```

The visual complexity comes from composing transformations, not necessarily
from making any single transformation complicated.

When a shader effect looks difficult, ask which coordinate transformation
could make the underlying shape simple. A wave may be a straight line in a
warped domain. A twist may be a circle evaluated after position-dependent
rotation. A complicated repeated pattern may be one primitive evaluated after
domain repetition.

That is the central technique of domain warping: change the space in which
the simple mathematical model lives.

## Next step

Now type the code version of this lesson.

