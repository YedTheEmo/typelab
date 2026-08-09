````markdown
# Coordinate spaces and transforms - concepts

A coordinate is only meaningful relative to the space in which it is
interpreted. The numbers `(1, 0, 0)` can describe a point one unit to the
right of an object's origin, one unit to the right of the world origin, or
one unit to the right of the camera. The numbers are identical, but the
geometric meaning is different.

This is why graphics programs use multiple coordinate spaces. An object can
be defined in a simple local coordinate system, placed into the scene by a
transform, viewed from a camera, and eventually mapped toward the screen.
Each step changes the representation of the same geometry.

For shader programming, this idea has another important consequence: instead
of changing a mathematical object, we can transform the coordinates before
evaluating the object. This becomes one of the most important techniques in
procedural graphics.

## A coordinate needs a reference frame

Suppose an object contains a point:

```
float3 p = float3(1.0, 0.0, 0.0);
```

By itself, this only tells us three numbers.

If those coordinates belong to the object, they mean the point is one unit
along the object's local X axis.

If those coordinates belong to the world, they mean the point is one unit
along the world's X axis.

The point has not necessarily changed. Its representation has.

This gives us a useful mental model:

```
geometry = the thing being described
coordinates = how that thing is represented in a space
```

A coordinate transform changes the second thing.

## Object space is the object's local world

Object space is the coordinate system in which an object is defined.

Imagine a sphere whose center is the origin:

```
float3 center = float3(0.0, 0.0, 0.0);
float radius = 1.0;
```

Its mathematical description is convenient because the center does not need
to be stored separately. The sphere is simply centered around `(0, 0, 0)`.

Likewise, a mesh can be modeled around its own origin.

This is useful because the model does not need to know where it will appear in
the final scene. The same model can be placed at different positions by
changing its transform.

Object space therefore answers:

"Where is this point relative to this object?"

## World space provides a shared scene

World space gives objects a common reference frame.

Imagine two objects. Each can have a point at:

```
float3 p = float3(1.0, 0.0, 0.0);
```

That point can be identical in both objects' local spaces while the objects
themselves occupy different locations in the scene.

A transform moves the local coordinate into world coordinates:

```
float3 world = local + position;
```

Now the point has a meaning relative to the entire scene.

The important separation is that the object's geometry remains local while its
placement becomes a separate piece of information.

This is why a model can be reused many times without modifying its vertices.

## Translation moves coordinates

Translation is the simplest transformation.

Given a point `p` and an offset `t`:

```
float3 result = p + t;
```

The point moves by the amount stored in `t`.

For example:

```
float3 p = float3(1.0, 2.0, 0.0);
float3 t = float3(3.0, 0.0, 0.0);
float3 result = p + t;
```

The result is `(4, 2, 0)`.

Nothing about the point's orientation or size was changed. Every point is
simply displaced by the same amount.

This is why translation can move an entire object without changing its shape.

## Scale changes the coordinate's distance from the origin

Scaling changes how far coordinates are from their origin.

For uniform scale:

```
float3 result = p * 2.0;
```

Every coordinate is twice as far from the origin.

For nonuniform scale:

```
float3 result = p * float3(2.0, 1.0, 0.5);
```

each axis is scaled independently.

This distinction becomes especially useful for procedural geometry. A shape
function can remain unchanged while the coordinates entering it are scaled.

The function might describe a unit circle:

```
float distance = length(p) - 1.0;
```

If the input coordinates are scaled before that calculation, the resulting
shape can appear stretched.

The shape definition stays simple.

The space in which it is evaluated changes.

## Rotation changes orientation

Rotation changes where a coordinate points relative to an origin.

In two dimensions, rotation by an angle can be written as:

```
x' = x * cos(angle) - y * sin(angle)
y' = x * sin(angle) + y * cos(angle)
```

This is the same trigonometric relationship introduced in the previous
lesson.

We can express it as a function:

```
float2 rotate(float2 p, float angle)
{
    return float2(
        p.x * cos(angle) - p.y * sin(angle),
        p.x * sin(angle) + p.y * cos(angle)
    );
}
```

A point at `(1, 0)` rotated by 90 degrees becomes approximately `(0, 1)`.

The distance from the origin stays the same. Rotation changes orientation,
not magnitude.

This is an important property of transformations: different transforms
preserve or change different geometric relationships.

## Transform order matters

Transforms can be composed, but their order matters.

Suppose a point is first scaled and then translated:

```
p -> scale -> translate
```

The scale changes the point relative to the origin, and the translation then
moves the scaled result.

Now reverse the operations:

```
p -> translate -> scale
```

The translation itself is also scaled.

These two sequences therefore produce different results.

For example:

```
float3 p = float3(1.0, 0.0, 0.0);
float3 scaled = p * 2.0;
float3 result = scaled + float3(5.0, 0.0, 0.0);
```

produces a different point from:

```
float3 moved = p + float3(5.0, 0.0, 0.0);
float3 result = moved * 2.0;
```

The first sequence gives `(7, 0, 0)`.

The second gives `(12, 0, 0)`.

Transform composition is therefore not simply a list of interchangeable
operations. The order defines the geometry.

## Matrices package transformations together

Translation, rotation, and scale can each be represented mathematically.
Matrices provide a common representation that lets transformations be
combined.

A point is commonly extended to four components:

```
float4 p = float4(position, 1.0);
```

The fourth component allows translation to participate in matrix
multiplication.

A transformation can then be expressed conceptually as:

```
float4 transformed = matrix * p;
```

The matrix contains the information required to transform the coordinate.

A chain of transformations can also be combined:

```
object -> world -> view -> projection
```

Rather than manually performing every operation at every use site, a graphics
program can construct matrices representing those transformations and apply
them as part of the rendering pipeline.

The exact matrix conventions depend on the graphics API and mathematical
convention, but the underlying idea remains the same: a matrix is a compact
way to describe how coordinates should be transformed.

## The rendering pipeline changes spaces

A conventional 3D rendering pipeline can be understood through a sequence of
coordinate spaces:

```
object -> world -> view -> clip
```

Object space describes geometry relative to its object.

World space describes that geometry relative to the scene.

View space describes the scene relative to the camera.

Clip space is the representation used by the rendering pipeline before
rasterization continues toward the screen.

The same vertex travels through these representations.

It is not being recreated at every stage. Its coordinates are being
transformed so that each stage can reason about it using a useful reference
frame.

## View space makes the camera the reference

A camera is another coordinate frame.

A world-space point can be transformed into view space:

```
float4 viewPosition = worldToView * worldPosition;
```

The resulting coordinates describe the point relative to the camera.

This is useful because many camera-related calculations become easier when
the camera is treated as the origin of the coordinate system.

Moving the camera therefore does not require changing every object in the
world. Instead, the transformation that converts world coordinates into
camera coordinates changes.

The scene remains in world space.

The camera changes the representation.

## Projection changes how 3D appears

A view-space point still describes a location in three dimensions. Rendering
needs a representation suitable for projecting that location toward a
two-dimensional image.

Projection performs this transformation.

Conceptually:

```
view space -> projection -> clip space
```

Perspective projection introduces the familiar relationship between distance
and apparent size: points farther away from the camera occupy less screen
space.

Orthographic projection does not introduce the same perspective shrinking.

The details of projection matrices belong to the graphics pipeline, but the
important mathematical idea is simple: projection is another transformation
of coordinates.

The shader does not need a special concept called "perspective" to move a
point. Perspective is encoded in the mathematical transformation applied to
that point.

## The fourth component carries extra information

After projection, coordinates are commonly represented using four components:

```
float4 clipPosition;
```

The fourth component, commonly called `w`, participates in the next stage of
the coordinate transformation.

A perspective divide conceptually looks like:

```
float3 ndc = clipPosition.xyz / clipPosition.w;
```

This division is what turns the projected representation into normalized
device coordinates.

The important point is not to memorize the pipeline as a collection of
special graphics rules. The useful mental model is that the representation
continues to change through mathematical operations until it is suitable for
rasterization.

## Procedural geometry uses the same idea

Coordinate transforms become even more interesting when there is no mesh.

Suppose a sphere is defined by:

```
float sphere(float3 p)
{
    return length(p) - 1.0;
}
```

The function assumes the sphere is centered at the origin.

Suppose we want that sphere centered somewhere else.

We could add a center parameter to the function, but there is another approach:

```
float3 local = p - center;
float distance = sphere(local);
```

We moved the coordinate into the sphere's local space before evaluating it.

The sphere function remains simple.

This is a general pattern:

```
world coordinate
    -> local coordinate
    -> mathematical object
```

The object does not need to know where it lives in the world.

## Transform the input instead of the object

This leads to one of the most important ideas for procedural shader work.

Suppose:

```
d = shape(p)
```

describes some mathematical object.

We can instead write:

```
d = shape(transform(p))
```

The shape function stays unchanged while the input space changes.

If `transform` translates the coordinate, the shape appears translated.

If it rotates the coordinate, the shape appears rotated.

If it scales the coordinate, the shape appears scaled.

The apparent transformation happens because every point is being evaluated in a
different local coordinate system.

This technique is called transforming the domain.

It becomes especially powerful when several transformations are composed
before evaluating a simple shape.

## Coordinate spaces are tools for reasoning

A coordinate-space bug is often not a failure of mathematics. It is a failure
to agree on what the numbers mean.

A position in world space should not casually be combined with a position in
object space.

A normal transformed into one space should not be compared against a light
direction expressed in another.

A procedural shape expecting local coordinates should not receive arbitrary
world coordinates unless that is intentional.

Whenever a shader calculation looks mathematically correct but produces an
incorrect visual result, one useful question is:

"What coordinate space is this value in?"

That question often reveals the problem.

## The central mental model

Coordinate spaces are different reference frames.

Transforms convert coordinates between those frames.

The conventional rendering chain is:

```
object -> world -> view -> clip
```

Procedural graphics frequently use another form:

```
world -> local -> mathematical function
```

The most important technique is to recognize that transforming the coordinates
going into a function can transform the apparent object produced by that
function.

Once this becomes intuitive, matrices stop being mysterious collections of
numbers. They become tools for answering a geometric question:

"How should this coordinate be represented in another space?"

## Next step

Now type the code version of this lesson.
````

