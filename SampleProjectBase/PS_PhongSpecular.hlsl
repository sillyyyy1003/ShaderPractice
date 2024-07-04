//ピクセルシェーダの入力
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : POSITION0;
};

//ライト情報を保持する定数バッファ
cbuffer Light : register(b0)
{
    float4 lightDiffuse;    //ライトの拡散反射光
    float4 lightAmbient;    //ライトの環境光
    float4 lightDir;        //ライトの方向
}

//カメラの定数バッファ
cbuffer Camera : register(b1)
{
    float4 camPos;//カメラの位置
}

//テクスチャのバインド
Texture2D tex : register(t0);

//サンプラーのバインド
SamplerState samp : register(s0);

//ピクセルシェーダのメイン処理
//頂点で囲まれているピクセル分だけ実行する
float4 main (PS_IN pin) : SV_TARGET
{
    float4 color;
    //テクスチャから色を取得
    color = tex.Sample(samp, pin.uv);
    
    float3 N = normalize(pin.normal);//法線ベクトルを正規化
    float3 L = normalize(-lightDir.xyz);//ライトの方向を正規化
    float diffuse = saturate(dot(N, L));//拡散反射光の計算（法線ベクトルとライトの方向の内積）
    
    color *= diffuse * lightDiffuse + lightAmbient;//カラーに拡散反射光と環境光を適用
    
    //視線ベクトルと反射ベクトルの計算
    float3 V = normalize(camPos.xyz - pin.worldPos);
    float3 R = normalize(reflect(-L, N));
    
    //鏡面光の計算
    float specular = saturate(dot(V, R));
    //カラーに鏡面光を加算
    color += pow(specular, 5.0f);
    //アルファ値を1.0に設定
    color.a = 1.0f;
    
    return color;
}