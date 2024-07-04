struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

cbuffer Light : register(b0)
{
    float4 lightDiffuse;
    float4 lightAmbient;
    float4 lightDir;
};
cbuffer Camera : register(b1)
{
    float4 camPos;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	// �e�N�X�`������F���擾
    color = tex.Sample(samp, pin.uv);

    float3 N = normalize(pin.normal); // �@���x�N�g���𐳋K��
    float3 L = normalize(-lightDir.xyz); // ���C�g�̕����𐳋K��
    float diffuse = saturate(dot(N, L)); // �g�U���ˌ��̌v�Z(�@���x�N�g���ƃ��C�g�̕����̓���)

    color *= diffuse * lightDiffuse + lightAmbient; //�J���[�Ɋg�U���ˌ��Ɗ�����K�p

    float3 V = normalize(camPos.xyz - pin.worldPos);
    float lv = saturate(-dot(L, V));

    float edge = 1.3f - saturate(dot(N, V));
    float rim = lv * pow(edge, 5.0f);

    color += rim;
    

    return color;
}
