# Shader overview - typing

This lesson types a parallel image calculation: identify an invocation,
convert its position into coordinates, and turn those coordinates into color.

## Identify the invocation

A compute shader receives a position identifying the current work item.

```
// describe the output image
RWTexture2D<float4> outputImage;

// describe the dimensions of the image
uint2 imageSize;

// run this function across many work items
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the integer position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // normalize the position into the image coordinate range
    float2 uv = pixel / float2(imageSize);

    // store the coordinate as visible color
    outputImage[dispatchThreadID.xy] = float4(uv, 0.0, 1.0);
}
```

## Calculate distance

A coordinate can become an input to a geometric calculation.

```
// describe the output image
RWTexture2D<float4> outputImage;

// describe the dimensions of the image
uint2 imageSize;

// describe the center of the effect
float2 center;

// run this function across many work items
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the integer position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // normalize the position into the image coordinate range
    float2 uv = pixel / float2(imageSize);

    // measure the coordinate's distance from the center
    float distance = length(uv - center);

    // turn distance into a decreasing brightness value
    float brightness = 1.0 - distance;

    // store the brightness as a grayscale color
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

## Turn a value into a shape

A mathematical value can decide whether a location belongs to a region.

```
// describe the output image
RWTexture2D<float4> outputImage;

// describe the dimensions of the image
uint2 imageSize;

// describe the center of the effect
float2 center;

// describe the radius of the region
float radius;

// run this function across many work items
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the integer position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // normalize the position into the image coordinate range
    float2 uv = pixel / float2(imageSize);

    // measure the coordinate's distance from the center
    float distance = length(uv - center);

    // determine whether this location is inside the radius
    float inside = distance < radius ? 1.0 : 0.0;

    // turn the membership value into a visible color
    outputImage[dispatchThreadID.xy] =
        float4(inside, inside, inside, 1.0);
}
```

## Make the shape continuous

Instead of making a hard decision, the same distance can produce a smooth
value.

```
// describe the output image
RWTexture2D<float4> outputImage;

// describe the dimensions of the image
uint2 imageSize;

// describe the center of the effect
float2 center;

// run this function across many work items
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert the integer position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // normalize the position into the image coordinate range
    float2 uv = pixel / float2(imageSize);

    // measure the coordinate's distance from the center
    float distance = length(uv - center);

    // convert distance into a soft brightness value
    float brightness = saturate(1.0 - distance);

    // store the brightness as a grayscale color
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

## Apply the same rule everywhere

The important part is that the shader does not construct each pixel separately.

```
// describe the output image
RWTexture2D<float4> outputImage;

// describe the dimensions of the image
uint2 imageSize;

// describe the center of the effect
float2 center;

// run the same mathematical program across the image
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert this invocation's position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // map the position into a resolution-independent coordinate space
    float2 uv = pixel / float2(imageSize);

    // calculate the vector from the center to this invocation
    float2 offset = uv - center;

    // calculate how far this invocation is from the center
    float distance = length(offset);

    // map distance into a visible scalar value
    float brightness = saturate(1.0 - distance);

    // write this invocation's result to its own image location
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

## Now type it again

Type the complete mathematical path again without looking back.

```
// describe the output image
RWTexture2D<float4> outputImage;

// describe the dimensions of the image
uint2 imageSize;

// describe the center of the effect
float2 center;

// run the same mathematical program across the image
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // convert this invocation's position into floating point coordinates
    float2 pixel = float2(dispatchThreadID.xy);

    // map the position into a resolution-independent coordinate space
    float2 uv = pixel / float2(imageSize);

    // calculate the vector from the center to this invocation
    float2 offset = uv - center;

    // calculate how far this invocation is from the center
    float distance = length(offset);

    // map distance into a visible scalar value
    float brightness = saturate(1.0 - distance);

    // write this invocation's result to its own image location
    outputImage[dispatchThreadID.xy] =
        float4(brightness, brightness, brightness, 1.0);
}
```

The flow:

```
invocation -> coordinate -> offset -> distance -> brightness -> color
```

## Wrap up

A shader applies one mathematical rule across many independent invocations.
Coordinates identify where an invocation is operating, and mathematical
functions turn those coordinates into values that can become visible output.

