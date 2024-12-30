#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 fragColor [[color(0)]];
};

struct main0_in
{
    float4 color [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.fragColor = in.color;
    return out;
}

