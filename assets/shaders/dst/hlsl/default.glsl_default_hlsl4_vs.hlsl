cbuffer vs_params : register(b0)
{
    row_major float4x4 _19_projectionMatrix : packoffset(c0);
    row_major float4x4 _19_viewMatrix : packoffset(c4);
    row_major float4x4 _19_modelMatrix : packoffset(c8);
};


static float4 gl_Position;
static float gl_PointSize;
static float3 vertexPosition;
static float2 tcoord;
static float2 vertexTCoord;
static float4 color;
static float4 vertexColor;
static float3 vertexNormal;

struct SPIRV_Cross_Input
{
    float3 vertexPosition : TEXCOORD0;
    float2 vertexTCoord : TEXCOORD1;
    float4 vertexColor : TEXCOORD2;
    float3 vertexNormal : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float2 tcoord : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(float4(vertexPosition, 1.0f), mul(_19_modelMatrix, mul(_19_viewMatrix, _19_projectionMatrix)));
    tcoord = vertexTCoord;
    color = vertexColor;
    gl_PointSize = 1.0f;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vertexPosition = stage_input.vertexPosition;
    vertexTCoord = stage_input.vertexTCoord;
    vertexColor = stage_input.vertexColor;
    vertexNormal = stage_input.vertexNormal;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.tcoord = tcoord;
    stage_output.color = color;
    return stage_output;
}
