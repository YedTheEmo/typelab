# Shape blending and morphing - concepts

The previous lesson changed our representation of geometry.

Instead of thinking of a shape as a collection of vertices, we represented it
as a function that takes a point and returns a signed distance:

```text
point -> distance
```

The surface exists where the distance is zero.

This gives us something more powerful than a description of one isolated
shape. We now have a scalar value that can be mathematically combined with
other scalar values.

That means geometry itself can be combined through mathematics.

Two distance fields can become one larger shape. A shape can be subtracted from
another. Two surfaces can be blended together so that they smoothly merge.
Two different fields can even be interpolated so that one shape gradually
becomes another.

The important transition is:

```text
primitive shapes
    ↓
distance fields
    ↓
scalar operations
    ↓
new geometry
```

## Geometry becomes arithmetic

Suppose two shapes are evaluated at the same point:

```text
float a = shapeA(p);
float b = shapeB(p);
```

We now have two numbers describing the relationship between the point and two
different surfaces.

For example:

```text
a = -0.2
b =  0.7
```

The point is inside the first shape and outside the second.

Because both values describe the same point, we can compare them.

The simplest operation is:

```text
min(a, b)
```

This selects the smaller distance.

For SDF-style fields, that corresponds to taking the union of the two shapes.

The important concept is not memorizing `min` as a magic SDF operator.

The important concept is that a geometric operation has become an operation
on scalar fields.

## Union

Imagine two circles that overlap.

Each circle has its own distance field:

```text
a = circleA(p)
b = circleB(p)
```

To represent the region belonging to either circle, we use:

```text
d = min(a, b)
```

At every point, whichever shape is closer supplies the resulting field.

The zero region therefore covers both shapes.

Conceptually:

```text
A OR B
```

becomes:

```text
min(A, B)
```

This is the first major example of constructive geometry.

We are no longer manually describing the combined outline.

We are combining the mathematical descriptions of the shapes.

## Intersection

Union asks:

"Is the point inside either shape?"

Intersection asks:

"Is the point inside both shapes?"

For signed distance fields, the corresponding operation is:

```text
d = max(a, b)
```

Why?

Inside a shape means its distance is negative.

Suppose:

```text
a = -0.4
b = -0.2
```

The point is inside both shapes.

The maximum is:

```text
max(-0.4, -0.2) = -0.2
```

so the result remains negative.

Now suppose:

```text
a = -0.4
b = 0.5
```

The point is inside A but outside B.

The maximum is:

```text
max(-0.4, 0.5) = 0.5
```

so the result is outside the intersection.

Thus:

```text
union        -> min(a, b)
intersection -> max(a, b)
```

These operations are not arbitrary tricks. They follow from what the sign of
the distance means.

## Subtraction

We can also remove one shape from another.

Suppose we want:

```text
A - B
```

We want points inside A to remain, except where B occupies them.

The SDF operation can be expressed as:

```text
d = max(a, -b)
```

Negating `b` reverses its inside/outside relationship.

A point that was inside B becomes positive after negation, while a point
outside B becomes negative.

Taking the maximum with A then removes the region occupied by B.

This lets us construct forms such as:

```text
large shape - small shape
```

which can create holes, cavities, cuts, and other subtractive forms.

Again, the geometry is being constructed through arithmetic on fields.

## Boolean operations are hard boundaries

The `min` and `max` operations have an important visual characteristic.

Suppose two spheres overlap:

```text
d = min(a, b)
```

The result switches abruptly from one field to the other.

On one side of the transition, the value comes from A.

On the other side, it comes from B.

There is no transition region between them.

The surfaces therefore remain visibly distinct except where the boolean
operation requires them to meet.

This is useful when we actually want a hard union, intersection, or cut.

But many organic forms do not have hard boundaries.

A blob of clay does not look like two spheres whose surfaces suddenly meet.

We need a way to smoothly combine the fields.

## Smooth minimum

A smooth minimum replaces the abrupt transition of `min` with a continuous
blend.

Conceptually:

```text
hard union:

d = min(a, b)

smooth union:

d = smoothMin(a, b, k)
```

where `k` controls the width or strength of the transition.

When the two shapes are far apart, the result behaves approximately like the
ordinary minimum.

When they approach each other, the fields are gradually blended.

This creates the characteristic rounded connection between the shapes.

The key idea is that the operation changes the field near the boundary between
the two primitives rather than changing the primitives themselves.

## Why does smooth blending create a blob?

Imagine two spheres moving toward one another.

With:

```text
min(a, b)
```

each sphere retains its own surface.

When they overlap, the union simply chooses whichever sphere provides the
smaller field value.

With a smooth minimum, the values near the transition are modified so that
the field changes continuously from one primitive to the other.

The zero surface consequently bends through the transition region.

That bent zero surface is the visible "blob."

Nothing special has been added to the sphere equations.

The blob emerges from changing how their scalar fields are combined.

This is an important procedural-graphics principle:

**interesting geometry can emerge from simple functions combined in interesting
ways.**

## The blend parameter controls a region of influence

The smoothness parameter is not merely an artistic "blob amount."

It determines how large the region is in which the two fields are allowed to
interact smoothly.

A very small blending radius makes the result approach a hard union.

A larger radius causes the transition to extend farther around the meeting
region.

Conceptually:

```text
small k
    -> almost hard union

large k
    -> broad smooth connection
```

The exact formula used for a smooth minimum can vary.

The important thing to understand first is the role of the parameter: it
defines the spatial scale over which the two fields stop behaving like
independent surfaces and begin behaving like a blended field.

## Linear interpolation

Blending does not only apply to boolean operations.

We can also interpolate directly between two values.

The basic interpolation operation is linear interpolation, commonly written
as `lerp`.

Given two values `a` and `b`:

```text
lerp(a, b, t)
```

where:

```text
t = 0
```

produces `a`, and:

```text
t = 1
```

produces `b`.

Values between zero and one produce intermediate results.

Conceptually:

```text
t = 0.0 -> A
t = 0.25 -> mostly A
t = 0.5 -> halfway
t = 0.75 -> mostly B
t = 1.0 -> B
```

The mathematical form is:

```text
a + (b - a) * t
```

This is useful far beyond colors.

Any compatible numerical quantity can be interpolated.

## Interpolating distance fields

Because an SDF produces a scalar, we can interpolate two fields:

```text
d = lerp(a, b, t)
```

At `t = 0`, the field is A.

At `t = 1`, the field is B.

At intermediate values, the field is a mixture of the two.

This creates a useful distinction.

A smooth union combines shapes according to a spatial relationship between
their fields.

A linear interpolation combines the fields according to an external blend
parameter.

These are related ideas, but they are not the same operation.

Smooth union asks:

"How should these nearby shapes merge?"

Interpolation asks:

"How much of field A should become field B?"

## Morphing is changing one mathematical description into another

Suppose:

```text
a = sphere(p)
b = box(p)
```

We can define:

```text
d = lerp(a, b, t)
```

and animate `t` from zero to one.

At the beginning:

```text
t = 0
```

the field is the sphere field.

At the end:

```text
t = 1
```

the field is the box field.

Between them, the mathematical description continuously changes.

This is the basis of a simple procedural morph.

The visual result is not created by moving vertices from one mesh to another.
There may not be any vertices at all.

The shader simply evaluates a different scalar field as `t` changes.

That is a very different way of thinking about animation.

## Why the intermediate shape matters

A morph is not merely:

```text
show A
show B
```

It requires a continuous path through some mathematical space of possible
fields.

If:

```text
F(p, t)
```

is the field and `t` changes continuously, then every intermediate value of
`t` defines another field.

The renderer evaluates:

```text
F(p, 0.0)
F(p, 0.1)
F(p, 0.2)
...
F(p, 1.0)
```

The resulting surfaces can therefore evolve continuously.

This is another example of reducing a visual phenomenon to mathematics.

"Shape changing into another shape" becomes:

```text
one scalar function
    -> parameter
    -> another scalar function
```

## Blending and morphing are not identical

These concepts are easy to confuse because both involve combining values.

A smooth union is a **spatial combination** of shapes.

For example:

```text
two spheres
    -> smooth connection
```

The blend occurs because the fields interact near one another.

A morph is a **parameterized transition** between two descriptions.

For example:

```text
sphere
    -> intermediate forms
    -> box
```

The transition is controlled by a parameter such as `t`.

This distinction will remain useful later when procedural effects become more
complex.

## Interpolation can be controlled

Linear interpolation assumes the parameter changes linearly.

But visual transitions do not always need to progress linearly.

Suppose:

```text
t = 0.5
```

means exactly halfway through the mathematical transition.

We can pass the parameter through another function before using it:

```text
t2 = smoothstep(0.0, 1.0, t)
```

Now the transition begins gradually, accelerates, and then slows toward its
end.

The important principle is that interpolation and parameter shaping are
separate operations.

First decide:

```text
what values should be blended?
```

Then decide:

```text
how should the blend parameter evolve?
```

This separation becomes especially useful once time is introduced later in the
track.

## The field is the thing being manipulated

A common mistake is to think of the SDF as merely an intermediate step used to
produce a shape.

It is more useful to think of the field itself as the object being manipulated.

We can perform:

```text
sphere(p)
```

then:

```text
box(p)
```

then:

```text
min(sphere, box)
```

then:

```text
smoothMin(...)
```

then:

```text
lerp(...)
```

Each operation transforms the mathematical description of space.

The visible geometry is simply the zero surface of whatever field exists at the
end.

This gives us a powerful procedural pipeline:

```text
coordinates
    ↓
primitive fields
    ↓
field operations
    ↓
combined field
    ↓
zero surface
```

## Constructive geometry is mathematical composition

This approach is sometimes described as constructive solid geometry.

The important idea for shaders is that construction happens through functions.

Instead of saying:

"Take these vertices and manually create a new mesh."

we can say:

"Evaluate these fields and combine their values."

For example:

```text
sphere
    + sphere
    + subtraction
    + smooth union
```

can produce a complex object while each primitive remains mathematically
simple.

Complexity comes from composition.

That is one of the reasons procedural graphics can produce surprisingly rich
forms from relatively small programs.

## The coordinate transformation still matters

Everything from the previous lesson still applies.

A primitive can be evaluated in its own local space before being combined.

For example:

```text
a = sphere(p - centerA)
b = sphere(p - centerB)

d = smoothMin(a, b, k)
```

The spheres remain simple origin-centered functions.

Their positions come from transforming the input coordinates.

The combination then happens after both shapes have been expressed as fields
at the same point.

This separation between coordinate transformation and field combination is
what keeps procedural descriptions manageable.

## What makes an operation useful?

When constructing SDFs, a mathematical operation is useful when its behavior
has a geometric interpretation.

For example:

```text
min
```

can mean union.

```text
max
```

can mean intersection.

Negation can reverse inside and outside.

Interpolation can create a parameterized transition.

Smooth minimum can create a continuous connection.

The goal is therefore not to collect a library of mysterious formulas.

The goal is to understand what each operation does to the field and therefore
what it does to the zero surface.

## The central mental model

A primitive gives us a scalar field.

Two fields can be combined.

Hard operations such as `min` and `max` create boolean-style geometry.

Smooth operations modify the transition between fields.

Interpolation creates a continuous path from one mathematical description to
another.

The entire process can be summarized as:

```text
primitive
    ↓
distance field
    ↓
field operation
    ↓
new distance field
    ↓
zero surface
```

Once this becomes intuitive, procedural geometry stops looking like a
collection of isolated SDF tricks.

It becomes function composition.

The shape is whatever surface emerges from the final mathematical field.

## Next step

Now type the code version of this lesson.

```
```

