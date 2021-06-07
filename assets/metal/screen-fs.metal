// TODO... I don't know how to write this yet
#include <metal_stdlib>

struct params_t {
    float2 resolution;
    float2 scale;
    float2 viewport;
};

struct vs_in {
    float4 pos [[position]];
    float2 tcoord;
};

using namespace metal;
fragment float4 _main(vs_in in [[stage_in]],
                      constant params_t& params [[buffer(0)]],
                      texture2d<float, access::sample> texture [[texture(0)]],
                      sampler texSampler [[sampler(0)]]
) {
    //return in.color;
    //return float4(0.5);
    float2 uv = (in.pos.xy - params.viewport.xy) / params.resolution.xy * params.scale;
    float4 diffuseColor = texture.sample(texSampler, uv);
    return diffuseColor;
}

