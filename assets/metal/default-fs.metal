// TODO... I don't know how to write this yet
#include <metal_stdlib>
using namespace metal;

struct vs_in {
    float4 pos [[position]];
    float2 tcoord;
    float4 color;
};

fragment float4 _main(vs_in in [[stage_in]],
                      texture2d<float, access::sample> texture [[texture(0)]],
                      sampler texSampler [[sampler(0)]]
                                ) {
    float4 diffuseColor = texture.sample(texSampler, in.tcoord);
    return diffuseColor;
}

