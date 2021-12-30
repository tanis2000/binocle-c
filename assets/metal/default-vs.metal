#include <metal_stdlib>
using namespace metal;
struct params_t {
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 modelMatrix;
};
struct vs_in {
    float3 vertexPosition [[attribute(0)]];
    float4 vertexColor [[attribute(1)]];
    float2 vertexTCoord [[attribute(2)]];
    float3 vertexNormal [[attribute(3)]];
};
struct vs_out {
    float4 pos [[position]];
    float2 tcoord;
    float4 color;
};
vertex vs_out _main(vs_in in [[stage_in]], constant params_t& params [[buffer(0)]]) {
    vs_out out;
    out.pos = params.projectionMatrix * params.viewMatrix * params.modelMatrix * float4(in.vertexPosition, 1.0);
    out.tcoord = in.vertexTCoord;
    out.color = in.vertexColor;
    float3 n = in.vertexNormal;
    return out;
}

