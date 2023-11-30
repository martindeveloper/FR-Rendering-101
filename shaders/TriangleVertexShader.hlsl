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

float3x3 makeRotationZ(float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    return float3x3(
        c, -s, 0,
        s, c, 0,
        0, 0, 1
    );
}

VertexOutput main(VertexInput input)
{
    VertexOutput output;

    float scaledTime = time * 0.5;

    float pulse = (sin(scaledTime) + 1.0) * 0.5 + 0.5;

    float rotationAngle = scaledTime;
    float3x3 rotationMatrix = makeRotationZ(rotationAngle);

    float3 rotatedPosition = mul(input.position, rotationMatrix);
    output.position = float4(rotatedPosition * pulse, 1.0f);

    output.color = input.color;

    return output;
}
