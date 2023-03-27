cbuffer alphaBuffer : register(b0)
{
    float alpha;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    
    if (pixelColor.r == 0.0f && pixelColor.g == 0.0f && pixelColor.b == 0.0f)
    {
        discard;
    }
    
    return float4(pixelColor, alpha);
}