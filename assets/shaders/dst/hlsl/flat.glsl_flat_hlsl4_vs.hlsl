cbuffer vs_params : register(b0)
{
    row_major float4x4 _19_projectionMatrix : packoffset(c0);
    row_major float4x4 _19_viewMatrix : packoffset(c4);
    row_major float4x4 _19_modelMatrix : packoffset(c8);
};


static float4 gl_Position;
static float gl_PointSize;
static float3 vertexPosition;
static float4 color;
static float4 vertexColor;
static float2 vertexTexture;

struct SPIRV_Cross_Input
{
    float3 vertexPosition : TEXCOORD0;
    float4 vertexColor : TEXCOORD1;
    float2 vertexTexture : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 color : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(float4(vertexPosition, 1.0f), mul(_19_modelMatrix, mul(_19_viewMatrix, _19_projectionMatrix)));
    gl_PointSize = 1.0f;
    color = vertexColor;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vertexPosition = stage_input.vertexPosition;
    vertexColor = stage_input.vertexColor;
    vertexTexture = stage_input.vertexTexture;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.color = color;
    return stage_output;
}
