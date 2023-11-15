#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct fs_params
{
    float2 resolution;
    float2 scale;
    float2 viewport;
};

struct main0_out
{
    float4 fragColor [[color(0)]];
};

fragment main0_out main0(constant fs_params& _17 [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], sampler smp [[sampler(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out.fragColor = tex0.sample(smp, (((gl_FragCoord.xy - _17.viewport) / _17.resolution) * _17.scale));
    return out;
}

