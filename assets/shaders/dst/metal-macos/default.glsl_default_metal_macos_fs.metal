#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 fragColor [[color(0)]];
};

struct main0_in
{
    float2 tcoord [[user(locn0)]];
    float4 color [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> tex0 [[texture(0)]], sampler smp [[sampler(0)]])
{
    main0_out out = {};
    out.fragColor = in.color * tex0.sample(smp, in.tcoord);
    return out;
}

