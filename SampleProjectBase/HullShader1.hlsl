// �n���V�F�[�_�[�@�������@���`����V�F�[�_�[

// ���͐���_
// ControlPoint ... ����_�i���_�̂��Ɓj
// ���_�V�F�[�_�[����n�����f�[�^�̒�`
struct VS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// �o�͐���_
// �n���V�F�[�_�[����h���C���V�F�[�_�[��
// �n�����f�[�^�̒�`
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// �o�̓p�b�`�萔�f�[�^
// �p�b�` ... �������s���`��i���A�O�p�A�l�p
// �`�󂲂Ƃɗp�ӂ���f�[�^�����܂��Ă�i�h�L�������g�Q��
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]	: SV_TessFactor; // ���Ƃ��΁A�N���h �h���C���� [4] �ɂȂ�܂�
	float InsideTessFactor[2] : SV_InsideTessFactor; // ���Ƃ��΁A�N���h �h���C���� Inside[2] �ɂȂ�܂�
};

// �P�p�b�`�̒��_��
#define NUM_CONTROL_POINTS 4

// �p�b�`�萔�֐�
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// �O���̕�����
	Output.EdgeTessFactor[0] = 
	Output.EdgeTessFactor[1] = 
	Output.EdgeTessFactor[2] = 
	Output.EdgeTessFactor[3] = 100.0f;
	// �����̕�����
	Output.InsideTessFactor[0] =
	Output.InsideTessFactor[1] = 100.0f;

	return Output;
}


[domain("quad")] // �����`��̐ݒ�(isoline,tri,quad
[partitioning("fractional_odd")] // �������@(�A���S���Y��
[outputtopology("triangle_cw")] // �\�ʂ̐ݒ�
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// ���_�V�F�[�_�[�̃f�[�^���h���C���V�F�[�_�[�֓n��
	// �����Ńp�����[�^�̕ύX���s�Ȃ��Ă��悢
	Output.pos = ip[i].pos;
	Output.uv = ip[i].uv;

	return Output;
}
