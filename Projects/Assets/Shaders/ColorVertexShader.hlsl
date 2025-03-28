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
    vout.PosH = float4(vin.PosL, 1.0);
    vout.Color = vin.Color;
    return vout;
}