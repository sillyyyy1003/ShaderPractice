// �s�N�Z���V�F�[�_�̓���
struct PS_IN
{
    float4 pos : SV_POSITION; // �s�N�Z���̈ʒu
    float2 uv : TEXCOORD0; // �e�N�X�`�����W
    float3 normal : NORMAL; // �@���x�N�g��
};
// ���C�g����ێ�����萔�o�b�t�@
cbuffer Light : register(b0)
{
    float4 lightDiffuse; // ���C�g�̊g�U���ˌ�
    float4 lightAmbient; // ���C�g�̊���
    float4 lightDir; // ���C�g�̕���
};

// �J�����̒萔�o�b�t�@
cbuffer Camera : register(b1)
{
    float4 camPos; // �J�����̈ʒu
};

// �e�N�X�`���ւ̃o�C���h
Texture2D tex : register(t0);

// �T���v���[�ւ̃o�C���h
SamplerState samp : register(s0);

// �s�N�Z���V�F�[�_�̃��C������
// ���_�ň͂܂�Ă���s�N�Z�����������s����
float4 main(PS_IN pin) : SV_TARGET
{
    float4 color;

	// �e�N�X�`������F���擾
    color = tex.Sample(samp, pin.uv);

    float3 N = normalize(pin.normal); // �@���x�N�g���𐳋K��
    float3 L = normalize(-lightDir.xyz); // ���C�g�̕����𐳋K��
    float diffuse = saturate(dot(N, L)); // �g�U���ˌ��̌v�Z(�@���x�N�g���ƃ��C�g�̕����̓���)

    color *= diffuse * lightDiffuse + lightAmbient; //�J���[�Ɋg�U���ˌ��Ɗ�����K�p

    return color;
}
