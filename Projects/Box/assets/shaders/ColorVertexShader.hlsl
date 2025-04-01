cbuffer Transform : register(b0)
{
    float4x4 worldViewProjection;
};

struct VetexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut main(VetexIn vin)
{
    VertexOut vout;
    vout.PosH = mul(float4(vin.PosL, 1.0), worldViewProjection);
    vout.Color = vin.Color;
    return vout;
}