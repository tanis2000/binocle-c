#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct vs_params
{
    float4x4 transform;
};

struct main0_out
{
    float2 uvCoord [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant vs_params& _19 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _19.transform * float4(in.position, 1.0);
    out.uvCoord = (in.position.xy + float2(1.0)) * float2(0.5);
    return out;
}

