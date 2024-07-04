
static const float PI = 3.1415926f; // ��

// ���C�g�p�̒萔�o�b�t�@�[
cbuffer LightCb : register(b0)
{
    float4 lightDiffuse; // ���C�g�̊g�U���ˌ�
    float4 lightAmbient; // ���C�g�̊���
    float4 lightDir; // ���C�g�̕���
};

// �J�����p�̒萔�o�b�t�@�[
cbuffer CameraCb : register(b1)
{
    float4 eyePos; // �J�����̎��_
};

// �X�y�L�����p�̒萔�o�b�t�@�[
cbuffer SpecCb : register(b2)
{
    float specPow;  // �X�y�L�����̍i��
    float metallic; // ���^���b�N
    float smooth;   // �X���[�X
    float blank;    // ��
};

// ���_�V�F�[�_�[����󂯎�邽�߂̃f�[�^���`
struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 lightT : NORMAL1;
    float3 camVecT : NORMAL2;
    float4 worldPos : POSITION0;
};

// �e�N�X�`�����󂯎��
Texture2D tex : register(t0);
SamplerState samp : register(s0); // �e�N�X�`���̌J��Ԃ��ݒ�

// �@���}�b�v
Texture2D normalMap : register(t1);

// ���^���b�N�X���[�X�}�b�v�Br�Ƀ��^���b�N�Aa�ɃX���[�X ���Y���̃e�N�X�`�����Ȃ��̂Ŏg���Ȃ�
//Texture2D<float4> metallicSmoothMap : register(t2);

///////////////////////////////////////////////////
// �֐�
///////////////////////////////////////////////////

float3 GetNormal(float3 normal, float2 uv)
{
    float3 binSpaceNormal = normalMap.Sample(samp, uv);
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;
    
    return binSpaceNormal;
}

// �x�b�N�}�����z���v�Z����
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}

// �t���l�����v�Z�BSchlick�ߎ����g�p
float SpcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}

/// <summary>
/// Cook-Torrance���f���̋��ʔ��˂��v�Z
/// </summary>
/// <param name="L">�����Ɍ������x�N�g��</param>
/// <param name="V">���_�Ɍ������x�N�g��</param>
/// <param name="N">�@���x�N�g��</param>
/// <param name="metallic">�����x</param>
float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;

    // �����x�𐂒����˂̎��̃t���l�����˗��Ƃ��Ĉ���
    // �����x�������قǃt���l�����˂͑傫���Ȃ�
    float f0 = metallic;

    // ���C�g�Ɍ������x�N�g���Ǝ����Ɍ������x�N�g���̃n�[�t�x�N�g�������߂�
    float3 H = normalize(L + V);

    // �e��x�N�g�����ǂꂭ�炢���Ă��邩����ς𗘗p���ċ��߂�
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L)); 
    float NdotV = saturate(dot(N, V));

    // D�����x�b�N�}�����z��p���Čv�Z����
    float D = Beckmann(microfacet, NdotH);

    // F����Schlick�ߎ���p���Čv�Z����
    float F = SpcFresnel(f0, VdotH);

    // G�������߂�
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));

    // m�������߂�
    float m = PI * NdotV * NdotH;

    // �����܂ŋ��߂��A�l�𗘗p���āACook-Torrance���f���̋��ʔ��˂����߂�
    return max(F * D * G / m, 0.0);
}

/// <summary>
/// �t���l�����˂��l�������g�U���˂��v�Z
/// </summary>
/// <remark>
/// ���̊֐��̓t���l�����˂��l�������g�U���˗����v�Z���܂�
/// �t���l�����˂́A�������̂̕\�ʂŔ��˂��錻�ۂ̂Ƃ��ŁA���ʔ��˂̋����ɂȂ�܂�
/// ����g�U���˂́A�������̂̓����ɓ����āA�����������N�����āA�g�U���Ĕ��˂��Ă������̂��Ƃł�
/// �܂�t���l�����˂��ア�Ƃ��ɂ́A�g�U���˂��傫���Ȃ�A�t���l�����˂������Ƃ��́A�g�U���˂��������Ȃ�܂�
///
/// </remark>
/// <param name="N">�@��</param>
/// <param name="L">�����Ɍ������x�N�g���B���̕����Ƌt�����̃x�N�g���B</param>
/// <param name="V">�����Ɍ������x�N�g���B</param>
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    // step-4 �t���l�����˂��l�������g�U���ˌ������߂�

    // �@���ƌ����Ɍ������x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
    float dotNL = saturate(dot(N, L));

    // �@���Ǝ����Ɍ������x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
    float dotNV = saturate(dot(N, V));

    // �@���ƌ����ւ̕����Ɉˑ�����g�U���˗��ƁA�@���Ǝ��_�x�N�g���Ɉˑ�����g�U���˗���
    // ��Z���čŏI�I�Ȋg�U���˗������߂Ă���BPI�ŏ��Z���Ă���̂͐��K�����s������
    return (dotNL * dotNV);
}

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // �@�����v�Z
    float3 N = normalMap.Sample(samp, pin.uv);
 
    // ���O�Ɍv�Z���ꂽ�e�N�X�`����Ԃ̃��C�g
    float3 L = normalize(pin.lightT);
    float3 toLightVec = -L;
    
    // step-2 �e��}�b�v���T���v�����O����
    // �A���x�h�J���[�i�g�U���ˌ��j
    float4 albedoColor = tex.Sample(samp, pin.uv);

    // �X�y�L�����J���[�̓A���x�h�J���[�Ɠ����ɂ���
    float3 specColor = albedoColor;

    // �����x �e�N�X�`������p�����[�^�[���擾�ł���ꍇ�̓R�����g�A�E�g����
    //float metallic = metallicSmoothMap.Sample(samp, pin.uv).r;

    // ���炩�� �e�N�X�`������p�����[�^�[���擾�ł���ꍇ�̓R�����g�A�E�g����
    //float smooth = metallicSmoothMap.Sample(samp, pin.uv).a;

    // �����Ɍ������ĐL�т�x�N�g�����v�Z����
    float3 toEye = normalize(eyePos - pin.worldPos);

    float3 lig = 0;
    // step-3 �V���v���ȃf�B�Y�j�[�x�[�X�̊g�U���˂���������
    // �t���l�����˂��l�������g�U���˂��v�Z
    float diffuseFromFresnel = CalcDiffuseFromFresnel(N, -L, toEye);

    // ���K��Lambert�g�U���˂����߂�---------------------------------------
    //float NdotL = �@���ƃ��C�g�x�N�g���̓��ς��s��0�`1.0�͈̔͂ɂ���
    //float3 lambertDiffuse = ���̌��ʂɃ��C�g�̃J���[���|���ĉ~�����Ő��K������
    //---------------------------------------------------------------------
    float NdotL = saturate(dot(N, toLightVec));
    float3 lambertDiffuse = lightAmbient * NdotL / PI;

    // �ŏI�I�Ȋg�U���ˌ����v�Z����
    float3 diffuse = albedoColor * diffuseFromFresnel * lambertDiffuse;

    // step-5 Cook-Torrance���f���𗘗p�������ʔ��˗����v�Z����
    // Cook-Torrance���f���̋��ʔ��˗����v�Z����
    float3 spec = CookTorranceSpecular(-L, toEye, N, smooth) * lightDiffuse;

    // �����x��������΁A���ʔ��˂̓X�y�L�����J���[�A�Ⴏ��Δ�
    // �X�y�L�����J���[�̋��������ʔ��˗��Ƃ��Ĉ���
    spec *= lerp(float3(1.0f, 1.0f, 1.0f), specColor, metallic);

    // step-6 ���炩�����g���āA�g�U���ˌ��Ƌ��ʔ��ˌ�����������
    // ���炩����������΁A�g�U���˂͎キ�Ȃ�
    lig += diffuse * (1.0f - smooth) + spec;
    
    // �����ɂ���グ
    lig += lightAmbient * albedoColor;
   
    color.xyz = lig;
    
	return color;
}