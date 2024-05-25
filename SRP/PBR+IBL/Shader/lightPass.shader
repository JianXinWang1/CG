Shader "ToyRP/lightpass"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        Cull Off ZWrite On ZTest Always

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"
            #include "UnityLightingCommon.cginc"

            #define PI 3.14159265358
            // F
            float3 F(float3 F0, float NV){
                float m = 1-NV;
                float m2 = m*m;
                return F0 + (1-F0)*m2*m2*m;
            }

            // D
            float D(float roughness, float NH){
                float alpha = roughness * roughness;
                float alpha2 = alpha*alpha;
                float NH2 = NH * NH;
                float m2 = (NH2*(alpha2-1)+1) * (NH2*(alpha2-1)+1);
                return alpha2/(PI*m2);
            }

            // G
            float G(float roughness, float NV, float NL){
                float alpha = roughness * roughness;
                float k = ((alpha+1)*(alpha+1))/8;
                float GGX1 =  NV/(NV*(1-k)+k);
                float GGX2 =  NL/(NL*(1-k)+k);
                return GGX1*GGX2;
            }

            float3 PBR(float3 albedo, float metallic, float roughness, float3 L,  float3 V, float3 N, float3 radiance){
                float3 H = normalize(L+V);
                float NdotL = max(dot(N, L), 0);
                float NdotV = max(dot(N, V), 0);
                float NdotH = max(dot(N, H), 0);
                float HdotV = max(dot(H, V), 0);
                float3 F0 = lerp(float3(0.04,0.04,0.04),albedo,metallic);
                float3 Fitem = F(F0,NdotV);
                float Ditem = D(roughness,NdotH);
                float Gitem = G(roughness,NdotV,NdotL);
                float3 ks = Fitem;
                float3 kd = (1-Fitem) * (1-metallic);
                float3 fSpecular = (Fitem * Ditem * Gitem) / (4 * NdotV * NdotL); 
                float3 fDiffuse = albedo/PI;

                fDiffuse*=PI;
                fSpecular*=PI;
                float3 color = (kd * fDiffuse + fSpecular) * radiance * NdotL;
                return color;
            }

            // 间接光照
            float3 IBL(
                float3 N, float3 V,
                float3 albedo, float roughness, float metallic,
                samplerCUBE _diffuseIBL, samplerCUBE _specularIBL, sampler2D _brdfLut)
            {

                float3 H = normalize(N);    // 用法向作为半角向量
                float NdotV = max(dot(N, V), 0);
                float HdotV = max(dot(H, V), 0);
                float3 R = normalize(reflect(-V, N));   // 反射向量

                float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

                float3 Fitem = F(F0,NdotV);
                float3 k_s = Fitem;
                float3 k_d = (1.0 - k_s) * (1.0 - metallic);

                // 漫反射
                float3 IBLd = texCUBE(_diffuseIBL, N).rgb;
                float3 diffuse = k_d * albedo * IBLd;

                // 镜面反射
                float MAX_MAPLEVEL = 6;

                float lod = 6 - roughness*6;
                float3 IBLs = texCUBElod(_specularIBL, float4(R, lod)).rgb;
                float2 brdf = tex2D(_brdfLut, float2(NdotV, roughness)).rg;
                float3 specular = IBLs * (F0 * brdf.x + brdf.y);

                float3 ambient = diffuse + specular;

                return ambient;
            }

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
            };

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.uv = v.uv;
                return o;
            }

            sampler2D _gdepth;
            sampler2D _GT0;
            sampler2D _GT1;
            sampler2D _GT2;
            sampler2D _GT3;
            sampler2D _lut;
            samplerCUBE _diffuse;
            samplerCUBE _specular;
            float4x4 _vpMatrix;
            float4x4 _vpMatrixInv;          

            fixed4 frag (v2f i,out float depthOut:SV_Depth) : SV_Target
            {
               
                float2 uv = i.uv;
                float4 GT2 = tex2D(_GT2, uv);
                float4 GT3 = tex2D(_GT3, uv);


                // 从 Gbuffer 解码数据
                float3 albedo = tex2D(_GT0, uv).rgb;
                float3 normal = tex2D(_GT1, uv).rgb * 2 - 1;
                float2 motionVec = GT2.rg;
                float roughness = GT2.b;
                float metallic = GT2.a;
                float3 emission = GT3.rgb;
                float occlusion = GT3.a;

                float d = UNITY_SAMPLE_DEPTH(tex2D(_gdepth, uv));
                float d_lin = Linear01Depth(d);

                depthOut = d;
                float4 ndcPos = float4(uv*2-1, d, 1);
                float4 worldPos = mul(_vpMatrixInv, ndcPos);
                worldPos /= worldPos.w;


                // 计算参数
                float3 N = normalize(normal);
                float3 L = normalize(float3(-500,500,-1000));
                float3 V = normalize(_WorldSpaceCameraPos.xyz - worldPos.xyz);
                float3 H = normalize(L+V);
                float3 radiance = float3(2.0,2.0,2.0);

                metallic = clamp(metallic,0.001,0.999);
                roughness = clamp(roughness,0.001,0.999);
                // 计算直接光照
                float3 color = PBR(albedo, metallic, roughness, L, V, N, radiance);
                color += emission;

                float3 iblColor = IBL(N,V,albedo,roughness,metallic,_diffuse,_specular,_lut);

                return float4(color + iblColor, 1);
            }
            ENDCG
        }
    }
}