cbuffer ConstantBuffer : register(b0)
{
    float time;
};

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;

    float scaledTime = time * 0.5;

    float pulse = (sin(scaledTime) + 1.0) * 0.5 + 0.5;

    output.position = float4(input.position * pulse, 1.0f);
    output.color = input.color;

    return output;
}
