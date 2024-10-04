struct VS_OUT
{
    float3 color:Color;
    float4 pos:SV_Position;
};

cbuffer Cbuf
{
    float4x4 transform;
};
VS_OUT main( float2 pos : POSITION, float3 color:Color )
{
    VS_OUT vs_out;
    vs_out.pos = mul(float4(pos.x, pos.y, 0.0f, 1.0f), transform);
    vs_out.color = float3(color.x, color.y, color.z);
    return vs_out;
}