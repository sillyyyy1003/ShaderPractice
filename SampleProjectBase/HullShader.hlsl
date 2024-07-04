// ���_�V�F�[�_�[����̎󂯎��
struct HS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
// �e�b�Z���[�V�����X�e�[�W�ւ̏o��
struct HS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// �p�b�`�萔�֐��o��
// �p�b�`...���b�V�����\������ʁi�T�[�t�F�X�j
struct HS_CONSTANT_OUT
{
	// �ӂ̕������̎w��A���̔z��̐��̓p�b�`�̌`��(��,�O�p,�l�p)�ɉ��������ƂȂ� 
	// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422455(v=vs.85)?redirectedfrom=MSDN
	float EdgeTessFactor[4]	: SV_TessFactor;
	// �����̕�����
	// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422453(v=vs.85)?redirectedfrom=MSDN
	float InsideTessFactor[2]	: SV_InsideTessFactor;
};
// �p�b�`�萔�֐�
// @param[in] input ���͂Ƃ��Ďg�p�ł��鐧��_�̔z��i���͂̌^���A�p�b�`���\�����钸�_���j
// @param[in] id �p�b�`ID
// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422338(v=vs.85)?redirectedfrom=MSDN
HS_CONSTANT_OUT CalcHSPatchConstants(
	InputPatch<HS_IN, 4> input,
	uint id : SV_PrimitiveID
) {
	HS_CONSTANT_OUT cout;
	cout.EdgeTessFactor[0] =
	cout.EdgeTessFactor[1] =
	cout.EdgeTessFactor[2] =
	cout.EdgeTessFactor[3] =
	cout.InsideTessFactor[0] =
	cout.InsideTessFactor[1] = 20.0f;
	return cout;
}



// �p�b�`�^�C�v�̎w��
// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422313(v=vs.85)
[domain("quad")]
// �������@
[partitioning("integer")]
// ������̃v���~�e�B�u�`��
[outputtopology("triangle_cw")]
// �o�͌�̃R���g���[���|�C���g��
[outputcontrolpoints(4)]
// �p�b�`�萔�֐��̎w��
[patchconstantfunc("CalcHSPatchConstants")]
// �n���V�F�[�_�[�̃G���g���|�C���g�i���O�Ƀn���V�F�[�_�[�ɕK�v�ȑ������w�肵�Ă���
HS_OUT main( 
	InputPatch<HS_IN, 4> hin,
	uint id : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID
) {
	HS_OUT hout;
	hout.pos = hin[id].pos;
	hout.uv = hin[id].uv;
	return hout;
}
