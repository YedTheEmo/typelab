````markdown
# Coordinate spaces and transforms - typing

This lesson types coordinate transforms: translation, rotation, scaling,
composition, and transforming coordinates before evaluating a shape.

## Translate a point

Translation moves a coordinate by adding an offset.

```slang
// store the original point
float3 point = float3(1.0, 2.0, 0.0);

// store the movement applied to the point
float3 translation = float3(3.0, -1.0, 0.0);

// move the point by the translation
float3 translated = point + translation;
```

## Scale a point

Scaling changes the distance of a point from the origin.

```slang
// store the original point
float3 point = float3(1.0, 2.0, 0.0);

// store a different scale for each axis
float3 scale = float3(2.0, 1.0, 0.5);

// scale the point around the origin
float3 scaled = point * scale;
```

## Rotate a coordinate

Rotation changes direction while preserving distance from the origin.

```slang
// store the point being rotated
float2 point = float2(1.0, 0.0);

// store the rotation angle
float angle = 1.5707963;

// calculate the cosine of the angle
float c = cos(angle);

// calculate the sine of the angle
float s = sin(angle);

// calculate the rotated horizontal component
float x = point.x * c - point.y * s;

// calculate the rotated vertical component
float y = point.x * s + point.y * c;

// combine the rotated components
float2 rotated = float2(x, y);
```

## Compose transformations

The order of transformations changes the result.

```slang
// store the original point
float3 point = float3(1.0, 0.0, 0.0);

// store the scale
float scale = 2.0;

// store the translation
float3 translation = float3(5.0, 0.0, 0.0);

// scale the point around the origin
float3 scaled = point * scale;

// translate the scaled point
float3 transformed = scaled + translation;
```

## Transform a procedural shape

A shape can be evaluated in a transformed local coordinate system.

```slang
// store the world-space point being evaluated
float3 worldPoint = float3(2.0, 0.5, 0.0);

// store the center of the local sphere
float3 sphereCenter = float3(1.0, 0.0, 0.0);

// move the world point into the sphere's local space
float3 localPoint = worldPoint - sphereCenter;

// measure the local point's distance from the sphere origin
float distance = length(localPoint);

// subtract the radius to obtain the sphere field
float sphereField = distance - 0.5;
```

## Transform the domain

The shape itself can stay unchanged while its input coordinates are modified.

```slang
// store the world-space point
float3 worldPoint = float3(2.0, 0.5, 0.0);

// store the object's local translation
float3 objectPosition = float3(1.0, 0.0, 0.0);

// move the evaluation point into object-local coordinates
float3 localPoint = worldPoint - objectPosition;

// evaluate the unchanged sphere equation
float sphereDistance = length(localPoint) - 0.5;

// convert the distance into a visible value
float brightness = saturate(1.0 - abs(sphereDistance) * 4.0);
```

## Now type it again

Type the complete transformation idea again without looking back.

```slang
// store the world-space point
float3 worldPoint = float3(2.0, 0.5, 0.0);

// store the object's position in world space
float3 objectPosition = float3(1.0, 0.0, 0.0);

// move the world point into the object's local space
float3 localPoint = worldPoint - objectPosition;

// store the local scale
float3 objectScale = float3(2.0, 1.0, 1.0);

// undo the object's scale in the local evaluation space
float3 scaledPoint = localPoint / objectScale;

// rotate the local coordinate
float angle = 0.5;

// calculate the cosine of the rotation
float c = cos(angle);

// calculate the sine of the rotation
float s = sin(angle);

// calculate the rotated horizontal component
float x = scaledPoint.x * c - scaledPoint.y * s;

// calculate the rotated vertical component
float y = scaledPoint.x * s + scaledPoint.y * c;

// construct the rotated local coordinate
float2 rotatedPoint = float2(x, y);

// calculate the distance from the transformed origin
float distance = length(rotatedPoint);

// evaluate a unit circle in the transformed space
float circleField = distance - 0.5;

// turn the field value into visible brightness
float brightness = saturate(1.0 - abs(circleField) * 5.0);
```

The flow:

```text
world point -> local space -> scale -> rotate -> shape function
```

## Wrap up

A transform changes the coordinate representation, and transforming the input
domain lets the same mathematical shape appear translated, scaled, or rotated.
```
````

