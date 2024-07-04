
static const float PI = 3.1415926f; // π

// ライト用の定数バッファー
cbuffer LightCb : register(b0)
{
    float4 lightDiffuse; // ライトの拡散反射光
    float4 lightAmbient; // ライトの環境光
    float4 lightDir; // ライトの方向
};

// カメラ用の定数バッファー
cbuffer CameraCb : register(b1)
{
    float4 eyePos; // カメラの視点
};

// スペキュラ用の定数バッファー
cbuffer SpecCb : register(b2)
{
    float specPow;  // スペキュラの絞り
    float metallic; // メタリック
    float smooth;   // スムース
    float blank;    // 空
};

// 頂点シェーダーから受け取るためのデータを定義
struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 lightT : NORMAL1;
    float3 camVecT : NORMAL2;
    float4 worldPos : POSITION0;
};

// テクスチャを受け取る
Texture2D tex : register(t0);
SamplerState samp : register(s0); // テクスチャの繰り返し設定

// 法線マップ
Texture2D normalMap : register(t1);

// メタリックスムースマップ。rにメタリック、aにスムース ※該当のテクスチャがないので使えない
//Texture2D<float4> metallicSmoothMap : register(t2);

///////////////////////////////////////////////////
// 関数
///////////////////////////////////////////////////

float3 GetNormal(float3 normal, float2 uv)
{
    float3 binSpaceNormal = normalMap.Sample(samp, uv);
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;
    
    return binSpaceNormal;
}

// ベックマン分布を計算する
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}

// フレネルを計算。Schlick近似を使用
float SpcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}

/// <summary>
/// Cook-Torranceモデルの鏡面反射を計算
/// </summary>
/// <param name="L">光源に向かうベクトル</param>
/// <param name="V">視点に向かうベクトル</param>
/// <param name="N">法線ベクトル</param>
/// <param name="metallic">金属度</param>
float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;

    // 金属度を垂直入射の時のフレネル反射率として扱う
    // 金属度が高いほどフレネル反射は大きくなる
    float f0 = metallic;

    // ライトに向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L + V);

    // 各種ベクトルがどれくらい似ているかを内積を利用して求める
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L)); 
    float NdotV = saturate(dot(N, V));

    // D項をベックマン分布を用いて計算する
    float D = Beckmann(microfacet, NdotH);

    // F項をSchlick近似を用いて計算する
    float F = SpcFresnel(f0, VdotH);

    // G項を求める
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));

    // m項を求める
    float m = PI * NdotV * NdotH;

    // ここまで求めた、値を利用して、Cook-Torranceモデルの鏡面反射を求める
    return max(F * D * G / m, 0.0);
}

/// <summary>
/// フレネル反射を考慮した拡散反射を計算
/// </summary>
/// <remark>
/// この関数はフレネル反射を考慮した拡散反射率を計算します
/// フレネル反射は、光が物体の表面で反射する現象のとこで、鏡面反射の強さになります
/// 一方拡散反射は、光が物体の内部に入って、内部錯乱を起こして、拡散して反射してきた光のことです
/// つまりフレネル反射が弱いときには、拡散反射が大きくなり、フレネル反射が強いときは、拡散反射が小さくなります
///
/// </remark>
/// <param name="N">法線</param>
/// <param name="L">光源に向かうベクトル。光の方向と逆向きのベクトル。</param>
/// <param name="V">視線に向かうベクトル。</param>
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    // step-4 フレネル反射を考慮した拡散反射光を求める

    // 法線と光源に向かうベクトルがどれだけ似ているかを内積で求める
    float dotNL = saturate(dot(N, L));

    // 法線と視線に向かうベクトルがどれだけ似ているかを内積で求める
    float dotNV = saturate(dot(N, V));

    // 法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
    // 乗算して最終的な拡散反射率を求めている。PIで除算しているのは正規化を行うため
    return (dotNL * dotNV);
}

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 法線を計算
    float3 N = normalMap.Sample(samp, pin.uv);
 
    // 事前に計算されたテクスチャ空間のライト
    float3 L = normalize(pin.lightT);
    float3 toLightVec = -L;
    
    // step-2 各種マップをサンプリングする
    // アルベドカラー（拡散反射光）
    float4 albedoColor = tex.Sample(samp, pin.uv);

    // スペキュラカラーはアルベドカラーと同じにする
    float3 specColor = albedoColor;

    // 金属度 テクスチャからパラメーターを取得できる場合はコメントアウト解除
    //float metallic = metallicSmoothMap.Sample(samp, pin.uv).r;

    // 滑らかさ テクスチャからパラメーターを取得できる場合はコメントアウト解除
    //float smooth = metallicSmoothMap.Sample(samp, pin.uv).a;

    // 視線に向かって伸びるベクトルを計算する
    float3 toEye = normalize(eyePos - pin.worldPos);

    float3 lig = 0;
    // step-3 シンプルなディズニーベースの拡散反射を実装する
    // フレネル反射を考慮した拡散反射を計算
    float diffuseFromFresnel = CalcDiffuseFromFresnel(N, -L, toEye);

    // 正規化Lambert拡散反射を求める---------------------------------------
    //float NdotL = 法線とライトベクトルの内積を行い0～1.0の範囲にする
    //float3 lambertDiffuse = ↑の結果にライトのカラーを掛けて円周率で正規化する
    //---------------------------------------------------------------------
    float NdotL = saturate(dot(N, toLightVec));
    float3 lambertDiffuse = lightAmbient * NdotL / PI;

    // 最終的な拡散反射光を計算する
    float3 diffuse = albedoColor * diffuseFromFresnel * lambertDiffuse;

    // step-5 Cook-Torranceモデルを利用した鏡面反射率を計算する
    // Cook-Torranceモデルの鏡面反射率を計算する
    float3 spec = CookTorranceSpecular(-L, toEye, N, smooth) * lightDiffuse;

    // 金属度が高ければ、鏡面反射はスペキュラカラー、低ければ白
    // スペキュラカラーの強さを鏡面反射率として扱う
    spec *= lerp(float3(1.0f, 1.0f, 1.0f), specColor, metallic);

    // step-6 滑らかさを使って、拡散反射光と鏡面反射光を合成する
    // 滑らかさが高ければ、拡散反射は弱くなる
    lig += diffuse * (1.0f - smooth) + spec;
    
    // 環境光による底上げ
    lig += lightAmbient * albedoColor;
   
    color.xyz = lig;
    
	return color;
}