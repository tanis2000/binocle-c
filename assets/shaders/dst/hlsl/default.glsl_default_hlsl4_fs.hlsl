Texture2D<float4> tex0 : register(t0);
SamplerState smp : register(s0);

static float2 tcoord;
static float4 fragColor;
static float4 color;

struct SPIRV_Cross_Input
{
    float2 tcoord : TEXCOORD0;
    float4 color : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 fragColor : SV_Target0;
};

void frag_main()
{
    fragColor = color * tex0.Sample(smp, tcoord);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    tcoord = stage_input.tcoord;
    color = stage_input.color;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.fragColor = fragColor;
    return stage_output;
}
