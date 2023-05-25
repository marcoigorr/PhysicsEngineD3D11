cbuffer particleBuffer : register(b0)
{
    float v_magnitude;
    float r_mod;
    float g_mod;
    float b_mod;
    bool enableColor;
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
    float4 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord);

    // rgb
    if (enableColor)
    {
        pixelColor[0] = v_magnitude * r_mod;
        pixelColor[1] = 1 - (v_magnitude * g_mod);
        pixelColor[2] = 1 - (v_magnitude * b_mod);
    }
        
    return pixelColor;
}