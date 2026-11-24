struct VSInput
{
    float3 inPosition;
    float4 inColor;
    float2 inTexCoord;
    uint inTexIndex;
};

struct UniformBuffer
{
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct PushConstants
{
    float4x4 model;
};
[[vk::push_constant]]
PushConstants push;

struct VSOutput
{
    float4 pos : SV_Position;
    float4 fragColor;
    float2 fragTexCoord;
    nointerpolation uint texIndex;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(push.model, float4(input.inPosition, 1.0))));
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    output.texIndex = input.inTexIndex;
    return output;
}

Sampler2D textures[];

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target
{
    return textures[NonUniformResourceIndex(vertIn.texIndex)].Sample(vertIn.fragTexCoord);
    // return vertIn.fragColor;
}

