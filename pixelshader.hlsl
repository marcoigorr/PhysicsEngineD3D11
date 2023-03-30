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
    float4 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord);

    // Debug: see alpha channel 
    /* pixelColor[0] = pixelColor[3];
    pixelColor[1] = pixelColor[2] = 0;
    pixelColor[3] = 1; */
    
    return pixelColor;
}