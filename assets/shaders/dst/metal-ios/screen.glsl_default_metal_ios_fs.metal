#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

static inline __attribute__((always_inline))
float2 uv_iq(thread const float2& uv, thread const int2& texture_size)
{
    float2 _20 = float2(texture_size);
    float2 _27 = floor(fma(uv, _20, float2(0.5)));
    return (_27 + fast::clamp(fma(uv, _20, -_27) / fwidth(uv * _20), float2(-0.5), float2(0.5))) / float2(texture_size);
}

fragment main0_out main0(constant fs_params& _56 [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], sampler smp [[sampler(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float2 param = ((gl_FragCoord.xy - floor(_56.viewport)) / _56.resolution) * _56.scale;
    int2 param_1 = int2(_56.resolution);
    out.fragColor = tex0.sample(smp, uv_iq(param, param_1));
    return out;
}

