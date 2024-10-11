#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct vs_params
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 modelMatrix;
};

struct main0_out
{
    float2 tcoord [[user(locn0)]];
    float4 color [[user(locn1)]];
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
};

struct main0_in
{
    float3 vertexPosition [[attribute(0)]];
    float4 vertexColor [[attribute(1)]];
    float2 vertexTCoord [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant vs_params& _19 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = ((_19.projectionMatrix * _19.viewMatrix) * _19.modelMatrix) * float4(in.vertexPosition, 1.0);
    out.tcoord = in.vertexTCoord;
    out.color = in.vertexColor;
    out.gl_PointSize = 1.0;
    return out;
}

