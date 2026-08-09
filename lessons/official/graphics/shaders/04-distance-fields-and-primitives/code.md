# Distance fields and primitives - typing

This lesson types SDF primitives: circles, spheres, planes, boxes, and the
signed distance values that describe their surfaces.

## Define a circle

A circle's signed distance is the distance from its center minus its radius.

```slang
// store the point being evaluated
float2 point = float2(0.7, 0.1);

// store the circle radius
float radius = 0.5;

// calculate the distance from the origin
float distance = length(point);

// move the zero crossing to the circle surface
float circleDistance = distance - radius;
```

## Define a sphere

The same distance relationship works in three dimensions.

```slang
// store the point being evaluated
float3 point = float3(0.7, 0.1, 0.2);

// store the sphere radius
float radius = 0.5;

// calculate the distance from the sphere center
float distance = length(point);

// move the zero crossing to the sphere surface
float sphereDistance = distance - radius;
```

## Move the sphere

Transform the input coordinate instead of changing the sphere equation.

```slang
// store the world-space point being evaluated
float3 point = float3(1.8, 0.1, 0.2);

// store the sphere's world-space center
float3 center = float3(1.0, 0.0, 0.0);

// store the sphere radius
float radius = 0.5;

// move the point into the sphere's local space
float3 localPoint = point - center;

// calculate the local distance from the sphere center
float distance = length(localPoint);

// produce the signed sphere distance
float sphereDistance = distance - radius;
```

## Define a plane

A horizontal plane can use its Y coordinate as signed distance.

```slang
// store the point being evaluated
float3 point = float3(1.0, 2.0, -3.0);

// read the signed distance above or below the plane
float planeDistance = point.y;

// convert the distance into a visible value
float brightness = saturate(1.0 - abs(planeDistance));
```

## Define a box

A box compares the point against its half-extents.

```slang
// store the point being evaluated
float3 point = float3(1.2, 0.4, 0.2);

// store half the box size on each axis
float3 halfSize = float3(1.0, 0.5, 0.5);

// fold the point into one symmetric region
float3 q = abs(point) - halfSize;

// measure distance outside the box
float outside = length(max(q, 0.0));

// measure penetration inside the box
float inside = min(max(q.x, max(q.y, q.z)), 0.0);

// combine the outside and inside distances
float boxDistance = outside + inside;
```

## Turn an SDF into a visual

The distance field can become a brightness value without changing the shape
function.

```slang
// store the point being evaluated
float2 point = float2(0.7, 0.1);

// store the circle radius
float radius = 0.5;

// calculate the signed distance to the circle
float distance = length(point) - radius;

// measure closeness to the zero surface
float brightness = exp(-abs(distance) * 20.0);
```

## Evaluate several primitives

Multiple distance fields can be calculated independently at the same point.

```slang
// store the point being evaluated
float3 point = float3(0.5, 0.2, 0.0);

// calculate the distance to the first sphere
float firstDistance = length(point - float3(-0.5, 0.0, 0.0)) - 0.4;

// calculate the distance to the second sphere
float secondDistance = length(point - float3(0.5, 0.0, 0.0)) - 0.4;

// keep the smaller field value
float combinedDistance = min(firstDistance, secondDistance);
```

## Now type it again

Type the core SDF pattern again from a clean start.

```slang
// store the world-space point being evaluated
float3 point = float3(1.2, 0.3, 0.0);

// store the sphere's world-space center
float3 center = float3(0.5, 0.0, 0.0);

// store the sphere radius
float radius = 0.5;

// move the point into the sphere's local coordinate space
float3 localPoint = point - center;

// calculate the distance from the sphere center
float distance = length(localPoint);

// convert the distance to a signed surface value
float sphereDistance = distance - radius;

// convert the signed field into a surface response
float surface = exp(-abs(sphereDistance) * 20.0);
```

The flow:

```text
point -> local space -> primitive equation -> signed distance -> visual value
```

## Wrap up

An SDF turns a point into a signed distance, making a primitive an implicit
mathematical surface whose zero set defines the geometry.
```
