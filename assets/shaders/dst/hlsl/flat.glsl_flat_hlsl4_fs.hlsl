static float4 fragColor;
static float4 color;

struct SPIRV_Cross_Input
{
    float4 color : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 fragColor : SV_Target0;
};

void frag_main()
{
    fragColor = color;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    color = stage_input.color;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.fragColor = fragColor;
    return stage_output;
}
