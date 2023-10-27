#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct vs_params
{
    float4x4 transform;
};

struct main0_out
{
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
    return out;
}

