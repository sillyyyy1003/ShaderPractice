// �h���C���V�F�[�_�[����s�N�Z���V�F�[�_�[�ɑ���f�[�^�̒�`
struct DS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
// �o�͐���_
// �n���V�F�[�_�[����h���C���V�F�[�_�[�ɓn�����f�[�^�̒�`
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
// �o�̓p�b�`�萔�f�[�^
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]	: SV_TessFactor;
	float InsideTessFactor[2] : SV_InsideTessFactor;
};


// �萔�o�b�t�@
cbuffer Matrix : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);


#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	// �l�p�`�̌v�Z
	// �z��̏��Ԃ͒��_�o�b�t�@���̒��_�̏��ԂɈˑ�����
	float4 edge[2] = {
		lerp(patch[1].pos, patch[2].pos, domain.x), // ��̕�
		lerp(patch[0].pos, patch[3].pos, domain.x)	// ���̕�
	};
	Output.pos = lerp(edge[0], edge[1], domain.y);
	float2 edgeUV[2] = {
		lerp(patch[1].uv, patch[2].uv, domain.x),
		lerp(patch[0].uv, patch[3].uv, domain.x)
	};
	Output.uv = lerp(edgeUV[0], edgeUV[1], domain.y);

	/* �O�p�`�̌v�Z
	�e���_�ɏd�݂�t���āA���Z�����l��������̒��_�ɂȂ�
	Output.vPosition = float4(
		patch[0].vPosition*domain.x + 
		patch[1].vPosition*domain.y + 
		patch[2].vPosition*domain.z,1);
		*/

	// �e�N�X�`���ɕ`�����܂ꂽ�l�ɉ����āA���_�̍�����ύX
	// �h���C���V�F�[�_�[�ł�Sample�����p�ł��Ȃ�
	// �����SampleLevel���g�p����
	float height = tex.SampleLevel(samp, Output.uv, 0).r;
	Output.pos.y += height * 0.3f;

	// ������̒��_��ϊ��s��ŕϊ�
	Output.pos = mul(Output.pos, world);
	Output.pos = mul(Output.pos, view);
	Output.pos = mul(Output.pos, proj);


	return Output;
}
