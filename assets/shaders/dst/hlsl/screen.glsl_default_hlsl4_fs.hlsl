cbuffer fs_params : register(b0)
{
    float2 _17_resolution : packoffset(c0);
    float2 _17_scale : packoffset(c0.z);
    float2 _17_viewport : packoffset(c1);
};

Texture2D<float4> tex0 : register(t0);
SamplerState smp : register(s0);

static float4 gl_FragCoord;
static float4 fragColor;

struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 fragColor : SV_Target0;
};

void frag_main()
{
    fragColor = tex0.Sample(smp, ((gl_FragCoord.xy - _17_viewport) / _17_resolution) * _17_scale);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.fragColor = fragColor;
    return stage_output;
}
