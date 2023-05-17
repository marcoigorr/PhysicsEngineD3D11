cbuffer particleBuffer : register(b0)
{
    double r_mod;
    double g_mod;
    double b_mod;
    double v_magnitude;
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
    pixelColor[0] = v_magnitude * r_mod;
    pixelColor[1] = 1 - (v_magnitude * g_mod);
    pixelColor[2] = 1 - (v_magnitude * b_mod);
    
    return pixelColor;
}