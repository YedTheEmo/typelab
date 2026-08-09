````markdown
# Vector intuition and trig - typing

This lesson types the core mathematical operations: displacement, distance,
normalization, directional alignment, periodic waves, and circular motion.

## Turn positions into displacement

Subtracting positions produces a vector pointing between them.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// store the target position
float2 target = float2(0.5, 0.5);

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the pixel position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // convert the pixel into normalized coordinates
    float2 uv = pixel / float2(imageSize);

    // calculate the vector from this pixel toward the target
    float2 toTarget = target - uv;

    // expose the vector components as image color
    outputImage[dispatchThreadID.xy] =
        float4(toTarget * 0.5 + 0.5, 0.0, 1.0);
}
```

## Calculate distance

The length of a displacement vector gives the distance to the target.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// store the target position
float2 target = float2(0.5, 0.5);

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the pixel position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // convert the pixel into normalized coordinates
    float2 uv = pixel / float2(imageSize);

    // calculate the vector from this pixel toward the target
    float2 toTarget = target - uv;

    // calculate the distance represented by that vector
    float distance = length(toTarget);

    // convert distance into a visible value
    float brightness = saturate(1.0 - distance * 2.0);

    // write the distance-based brightness
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

## Normalize a direction

Normalization keeps the direction while making the vector's length equal one.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// store the target position
float2 target = float2(0.5, 0.5);

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the pixel position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // convert the pixel into normalized coordinates
    float2 uv = pixel / float2(imageSize);

    // calculate the vector from this pixel toward the target
    float2 toTarget = target - uv;

    // remove the vector's magnitude while keeping its direction
    float2 direction = normalize(toTarget);

    // shift the signed direction into a visible color range
    float2 visibleDirection = direction * 0.5 + 0.5;

    // write the normalized direction as color
    outputImage[dispatchThreadID.xy] =
        float4(visibleDirection, 0.0, 1.0);
}
```

## Compare directions with a dot product

The dot product turns directional alignment into a scalar.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// define a direction pointing upward
float2 lightDirection = normalize(float2(0.2, 1.0));

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the pixel position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // convert the pixel into normalized coordinates
    float2 uv = pixel / float2(imageSize);

    // center the coordinate system around the image
    float2 centered = uv - 0.5;

    // calculate a radial direction from the image center
    float2 normal = normalize(centered);

    // measure how closely the directions align
    float alignment = dot(normal, lightDirection);

    // remove alignment values pointing away from the light
    float brightness = max(alignment, 0.0);

    // write the directional response
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

## Generate a periodic wave

Sine converts a continuously changing coordinate into smooth repeating values.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the pixel position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // convert the pixel into normalized coordinates
    float2 uv = pixel / float2(imageSize);

    // increase the spatial frequency of the wave
    float input = uv.x * 20.0;

    // calculate the periodic value
    float wave = sin(input);

    // map the sine range from negative-positive to zero-one
    float value = wave * 0.5 + 0.5;

    // write the wave as grayscale
    outputImage[dispatchThreadID.xy] =
        float4(value, value, value, 1.0);
}
```

## Use sine and cosine together

Sine and cosine can turn an angle into a direction around a circle.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// choose the current angular position
float angle = 1.0;

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // calculate the horizontal component of the circular direction
    float x = cos(angle);

    // calculate the vertical component of the circular direction
    float y = sin(angle);

    // combine the components into one direction vector
    float2 direction = float2(x, y);

    // shift the signed direction into a visible range
    float2 visibleDirection = direction * 0.5 + 0.5;

    // write the circular direction as color
    outputImage[dispatchThreadID.xy] =
        float4(visibleDirection, 0.0, 1.0);
}
```

## Build a circular position

A direction from sine and cosine can be scaled and moved to create a point
around a center.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// choose the center of the circle
float2 center = float2(0.5, 0.5);

// choose the circle radius
float radius = 0.25;

// choose the current angular position
float angle = 1.0;

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the angle into a unit direction
    float2 direction = float2(cos(angle), sin(angle));

    // scale the direction by the circle radius
    float2 offset = direction * radius;

    // move the offset from the origin to the circle center
    float2 point = center + offset;

    // convert the point into a visible color
    float3 color = float3(point.x, point.y, 0.0);

    // write the generated position
    outputImage[dispatchThreadID.xy] =
        float4(color, 1.0);
}
```

## Now type it again

Type the essential mathematical vocabulary again from a clean start.

```slang
// store the output image
RWTexture2D<float4> outputImage;

// store the dimensions of the image
uint2 imageSize;

// store the target position
float2 target = float2(0.5, 0.5);

// define a direction used for comparison
float2 referenceDirection = normalize(float2(0.5, 1.0));

// run the same calculation for many pixels
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the pixel position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // convert the pixel into normalized coordinates
    float2 uv = pixel / float2(imageSize);

    // calculate the displacement toward the target
    float2 offset = target - uv;

    // calculate the displacement magnitude
    float distance = length(offset);

    // keep the displacement direction while removing its magnitude
    float2 direction = normalize(offset);

    // compare the normalized directions
    float alignment = dot(direction, referenceDirection);

    // convert distance into a soft value
    float distanceValue = saturate(1.0 - distance * 2.0);

    // remove negative directional alignment
    float directionValue = max(alignment, 0.0);

    // combine the two mathematical measurements
    float brightness = distanceValue * directionValue;

    // write the combined result
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

The flow:

```text
positions -> displacement -> length
                       -> normalize -> dot
coordinates -> sine/cosine -> periodic or circular values
```

## Wrap up

Vectors describe spatial relationships, length extracts distance, normalization
isolates direction, dot products measure alignment, and trigonometric functions
turn continuous inputs into smooth periodic or circular behavior.
````

