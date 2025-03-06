cbuffer vs_params : register(b0)
{
    row_major float4x4 _19_transform : packoffset(c0);
};


static float4 gl_Position;
static float3 position;
static float2 uvCoord;

struct SPIRV_Cross_Input
{
    float3 position : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float2 uvCoord : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(float4(position, 1.0f), _19_transform);
    uvCoord = (position.xy + 1.0f.xx) * 0.5f.xx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    position = stage_input.position;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.uvCoord = uvCoord;
    return stage_output;
}
