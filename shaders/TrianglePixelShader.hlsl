cbuffer ConstantBuffer : register(b0)
{
    float time;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PixelInput input) : SV_TARGET
{
    float scaledTime = time * 0.5;
    float pulse = (sin(scaledTime) + 1.0) * 5;

    return input.color * pulse;
}
