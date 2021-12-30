#include <metal_stdlib>
using namespace metal;
struct params_t {
    float4x4 transform;
};
struct vs_in {
    float3 position [[attribute(0)]];
};
struct vs_out {
    float4 pos [[position]];
};
vertex vs_out _main(vs_in in [[stage_in]], constant params_t& params [[buffer(0)]]) {
    vs_out out;
    out.pos = params.transform * float4(in.position, 1.0);
    return out;
}

