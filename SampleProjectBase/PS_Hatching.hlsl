struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 worldPos : POSITION0;
};

// ���C�g�̏��
cbuffer Light : register(b0)
{
    float4 lightColor;
    float4 lightAmbient;
    float4 lightDir;
};

SamplerState samp : register(s0);
Texture2D tex : register(t0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = tex.Sample(samp, pin.uv);

	// y���W�����̂܂܏o�͂���ƁA�^�������Ȓ����ɂȂ�̂�
	// x���W�𗘗p���ėh�炬������
    float sideLineRandom = sin(pin.worldPos.x * 10.0f) * 10.0f;

	// ���f����y���W��sin�ɓn���āA�����I�Ȓ����Ƃ��ĕ\��
    float sideLine = sin(
		pin.worldPos.y * 1000.0f + // ���̖{����ύX
		sideLineRandom // ���̗h�炬
	);
    sideLine = step(-0.8f, sideLine); // ���̑�����ύX

	// �ʕ�������̒�����ǉ�
	// �Ȃ�ׂ������^�b�`�ɂ������̂Ŏ����p�����[�^��ݒ�
    float obliqueLine = sin(
		pin.worldPos.y * 1000.0f +
		pin.worldPos.x * 300.0f // x�����Ɍ������Ďΐ��ɂȂ�悤�␳
	);
    obliqueLine = step(-0.8f, obliqueLine);

	
	// ��{�I�ȉe�̌v�Z
    float3 N = normalize(pin.normal);
    float3 L = normalize(-lightDir.xyz);
    float dotNL = dot(N, L);

	// �G�̃^�b�`�ɂȂ�悤�ɁA�e�N�X�`�������m�N����
    color.rgb = color.r * 0.6f + color.g * 0.3f + color.b * 0.1f;

	// �e�̔Z���ɉ����āA�n�b�`���O���|����
    color.rgb = lerp(
		color.rgb * sideLine, // ���̃e�N�X�`���Ƀn�b�`���O��K�p
		color.rgb, // ���̃e�N�X�`��(�n�b�`���O�Ȃ�
		1.0f - pow(1.0 - saturate(dotNL), 2.0f)
	);

	// ���Â��Ȃ镔���ɁA����Ƀn�b�`���O��������
    color.rgb = lerp(
		color.rgb, // ���O�Ɍv�Z������r�I���邢�n�b�`���O
		color.rgb * obliqueLine, // �ΐ����d�˂��n�b�`���O
		1.0f - pow(1.0f - saturate(-dotNL), 3.0f)
	);

    return color;
}