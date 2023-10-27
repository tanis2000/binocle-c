cbuffer vs_params : register(b0)
{
    row_major float4x4 _19_transform : packoffset(c0);
};


static float4 gl_Position;
static float3 position;

struct SPIRV_Cross_Input
{
    float3 position : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(float4(position, 1.0f), _19_transform);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    position = stage_input.position;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
