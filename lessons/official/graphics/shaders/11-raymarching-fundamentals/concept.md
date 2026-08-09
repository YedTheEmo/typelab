# Raymarching fundamentals - concepts

Raymarching is a way to render a scene when the scene can be described by a
function that tells us how far a point is from the nearest surface.

This connects several ideas from earlier lessons. Coordinates describe the
point being evaluated, a distance field describes the geometry, and vectors
describe the camera ray. Raymarching combines them into an iterative search
through space.

The central idea is surprisingly simple: start at the camera, move along a
ray by the distance returned by the scene, and repeat.

## A ray is a point moving in a direction

A camera ray can be represented by an origin and a direction.

```slang
float3 rayOrigin = cameraPosition;
float3 rayDirection = normalize(pixelPosition - rayOrigin);
```

The origin is where the ray starts. The direction determines where it travels.

A point along that ray can be described using a scalar parameter:

```slang
float3 point = rayOrigin + rayDirection * distance;
```

When `distance` is zero, the point is at the camera. Increasing it moves the
point forward along the ray.

This equation is the foundation of raymarching.

## The scene is a distance function

An SDF maps a position to a scalar distance.

For a sphere centered at the origin:

```slang
float sphereSDF(float3 point, float radius) {
    return length(point) - radius;
}
```

A negative result means the point is inside the sphere. Zero means the point is
on its surface. A positive result means the point is outside.

The function therefore describes the geometry without storing triangles or
vertices.

Raymarching does not need to know how the sphere was constructed. It only needs
to evaluate the distance function.

## Why the distance is useful

Suppose the ray is currently at a point outside an object.

If the SDF says the nearest surface is five units away, then the ray can move
forward by five units without crossing that surface.

```slang
float distance = scene(point);
point += rayDirection * distance;
```

This is the defining property that makes a signed distance field useful for
raymarching.

The distance is not merely information about the geometry. It is also a safe
step size.

## The basic marching loop

The ray repeatedly evaluates the scene and advances by the returned distance.

```slang
float distance = 0.0;

for (int step = 0; step < maxSteps; step++) {
    float3 point = rayOrigin + rayDirection * distance;
    float sceneDistance = scene(point);
    distance += sceneDistance;
}
```

Each iteration asks a new question: "How far away is the nearest surface from
where I am now?"

The accumulated `distance` tells us how far the ray has traveled from the
camera.

Unlike fixed-step ray tracing, the ray does not move by an arbitrary constant
amount. Large empty regions can be crossed with large steps.

## Detecting a hit

The ray has reached a surface when the distance returned by the SDF becomes
very small.

```slang
if (sceneDistance < surfaceEpsilon) {
    break;
}
```

The shader cannot usually demand an exact zero. Floating-point calculations
and continuous functions make a small threshold more practical.

The epsilon therefore defines how close the ray must get before the shader
considers the surface found.

A smaller epsilon can produce more accurate intersections but may require more
iterations.

## Detecting a miss

A ray may never encounter a surface.

Without another termination condition, the marching loop could continue
indefinitely.

A maximum marching distance provides a second stopping condition:

```slang
if (distance > maxDistance) {
    break;
}
```

The ray therefore has two fundamental outcomes.

It can approach a surface closely enough to count as a hit, or it can travel
beyond the useful scene range and count as a miss.

A maximum number of steps is also necessary:

```slang
for (int step = 0; step < maxSteps; step++) {
    ...
}
```

This bounds the computational cost even when the ray encounters difficult
geometry.

## The complete raymarch

The central algorithm can now be written compactly.

```slang
float distance = 0.0;

for (int step = 0; step < maxSteps; step++) {
    float3 point = rayOrigin + rayDirection * distance;
    float sceneDistance = scene(point);

    if (sceneDistance < surfaceEpsilon) {
        break;
    }

    distance += sceneDistance;

    if (distance > maxDistance) {
        break;
    }
}
```

There are only a few operations here, but together they implement a complete
geometric search.

The ray position is reconstructed from its origin, direction, and accumulated
travel distance. The scene supplies the next safe step. The loop repeats until
the ray either reaches geometry or escapes.

## Why this differs from fixed-step sampling

A naive approach could move along the ray by a constant amount.

```slang
point += rayDirection * 0.01;
```

This guarantees that the ray samples the scene frequently, but it wastes work
in empty space and can miss thin geometry if the step is too large.

An SDF provides information about the nearest surface, allowing the step size
to adapt to the geometry.

```slang
point += rayDirection * sceneDistance;
```

This is why raymarching is often described as sphere tracing. At every point,
the distance field defines a sphere around the current position that is known
not to contain a surface.

The ray can advance to the edge of that safe region.

## The sphere-tracing mental model

Imagine placing a sphere around the current point with a radius equal to the
SDF value.

```slang
float safeRadius = scene(point);
```

If the distance function is a valid SDF, that sphere contains no surface.

The ray can therefore move forward by the sphere's radius.

At the new position, the shader constructs another safe sphere and repeats the
process.

The algorithm is not guessing where the surface is. It is repeatedly using
geometric distance information to guarantee that a sufficiently small step
will not cross the nearest surface.

## Combining objects into a scene

A scene can contain multiple SDF primitives.

For two objects, the union is represented by the minimum distance:

```slang
float first = sphereSDF(point, 0.7);
float second = sphereSDF(point - offset, 0.4);
float sceneDistance = min(first, second);
```

The minimum selects whichever object is closer to the current point.

This allows a complicated scene to still expose one distance function:

```slang
float scene(float3 point) {
    float first = sphereSDF(point, 0.7);
    float second = sphereSDF(point - offset, 0.4);
    return min(first, second);
}
```

Raymarching only needs this final scene distance. The internal construction of
the scene can remain modular.

## Transforming the domain

Objects do not need to be centered at the origin.

Instead of moving the object, the shader can transform the point before
evaluating its SDF:

```slang
float3 localPoint = point - objectPosition;
float distance = sphereSDF(localPoint, radius);
```

This is a useful mental model from coordinate-space mathematics: the primitive
can remain defined in its own local coordinate system while the input point is
converted into that system.

The same principle can support rotations, scaling, repetition, and other
domain transformations.

## Hit information

The raymarch needs to retain more than just the fact that a hit occurred.

The final ray distance can reconstruct the surface point:

```slang
float3 hitPoint = rayOrigin + rayDirection * distance;
```

That point can then be used for later calculations.

For example, the shader can estimate the surface normal from the distance
field and use the lighting mathematics from the previous lesson.

Raymarching therefore does not replace lighting. It provides a way to discover
the surface on which lighting will be evaluated.

## Estimating a normal

An SDF gives distance, not an explicit normal.

The normal can be approximated by measuring how the distance changes around the
surface.

For the x direction:

```slang
float dx = scene(hitPoint + float3(epsilon, 0.0, 0.0))
         - scene(hitPoint - float3(epsilon, 0.0, 0.0));
```

The same can be done for the other axes:

```slang
float3 normal = normalize(float3(dx, dy, dz));
```

This is a finite-difference approximation of the gradient.

The gradient points toward the direction in which the scalar field increases
most rapidly. For a signed distance field, that direction corresponds to the
surface normal.

This is an important connection between calculus and rendering.

## From distance to surface normal

The normal calculation can be expressed as a central difference.

```slang
float3 gradient = float3(
    scene(point + float3(epsilon, 0.0, 0.0))
        - scene(point - float3(epsilon, 0.0, 0.0)),
    scene(point + float3(0.0, epsilon, 0.0))
        - scene(point - float3(0.0, epsilon, 0.0)),
    scene(point + float3(0.0, 0.0, epsilon))
        - scene(point - float3(0.0, 0.0, epsilon))
);

float3 normal = normalize(gradient);
```

The SDF is sampled on both sides of the point along each axis.

Those differences estimate how the field changes in each coordinate direction.
Together they form the gradient vector.

The normal is then the normalized gradient.

## The camera ray

To render the image, every pixel needs its own ray.

A simple camera can construct a ray by placing the pixel into camera space and
normalizing the resulting direction:

```slang
float3 rayDirection = normalize(
    float3(screenPoint.x, screenPoint.y, focalLength)
);
```

Each pixel therefore evaluates the same raymarching algorithm with a different
direction.

This is where the GPU execution model becomes especially useful. The scene
function and marching loop are conceptually identical for every pixel, while
the input ray differs.

## Raymarching is per-pixel geometry evaluation

The complete conceptual structure is:

```slang
ray = camera(pixel);
distance = 0;

repeat:
    point = ray.origin + ray.direction * distance;
    step = scene(point);
    distance += step;
until hit or miss
```

A hit produces a surface point. The surface point can produce a normal. The
normal can feed the lighting equations.

The renderer has therefore reconstructed geometry procedurally at the moment
it needs it.

No polygon mesh is required for the sphere.

## Accuracy and performance

Raymarching has an important tradeoff between accuracy and computation.

A small surface epsilon allows the ray to approach the surface more closely,
but may require more iterations. A large epsilon can terminate too early and
make surfaces appear inaccurate.

The maximum step count places a hard limit on the amount of work per pixel.

The scene distance also matters. A poorly behaved distance function may report
a value larger than the true distance to a surface, causing the ray to step
too far and potentially skip geometry.

The quality of a raymarcher therefore depends not only on the loop, but also
on the mathematical properties of the distance function it evaluates.

## The deeper mental model

Raymarching turns a distance function into a search procedure.

The camera supplies a ray. The scene supplies a distance. That distance becomes
the next step. Repeating the process discovers whether and where the ray
intersects the scene.

Once a hit is found, the same SDF can provide a normal, and the normal can feed
the lighting equations from the previous lesson.

The complete chain is:

```slang
pixel -> ray -> scene distance -> march -> hit point -> normal -> lighting
```

The important conceptual shift is that the shader is no longer simply shading
geometry that was already supplied to it. It is using mathematics to discover
the geometry itself.

## Next step

Now type the code version of this lesson.

