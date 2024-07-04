struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 worldPos : POSITION0;
};

cbuffer Light : register(b0)
{
    float4 lightColor;
    float3 lightPos;
    float lightRange;
    float3 lightDir; // ���̕���
    float lightAngle; // �X�|�b�g���C�g�̐����p�x
};


Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	//�e�s�N�Z������_�����Ɍ������x�N�g�����v�Z
    float3 toLightVec = lightPos.xyz - pin.worldPos.xyz;
    float3 V = normalize(toLightVec); //�x�N�g�����K��
    float3 toLightLen = length(toLightVec); //�����܂ł̋���

	//�_�����Ɍ������x�N�g���Ɩ@�����疾�邳���v�Z
    float3 N = normalize(pin.normal);
    float dotNV = saturate(dot(N, V));

	//�����ɉ����Č��̋�����ς���
	//�������߂���Ζ��邭����Ă���΂��炭�Ȃ�悤�v�Z
    float3 attenuation = saturate(1.0f - toLightLen / lightRange);

	//���R�Ȍ����̌v�Z
	//�����ɉ����Č��̂�����ʂ���Z�ő����Ă���
    attenuation = pow(attenuation, 2.0f);

    float3 L = normalize(-lightDir);

    float dotVL = dot(V, L);
    float angle = acos(dotVL);

    //�p�x�ɉ����Ė��邳���v�Z�i1����O�ɃO���f�[�V�����ɂ��鏈���j
    float diffAngle = (lightAngle * 0.5f) * 0.1f;
    float spotAngle = ((lightAngle * 0.5f) + diffAngle);
    float spotRate = (spotAngle - angle) / diffAngle;
    spotRate = pow(saturate(spotRate), 2.0f);

    color.rgb = dotNV * attenuation * spotRate;
    return color;
}

