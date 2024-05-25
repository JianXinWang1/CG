Shader "Lit/Opaque"
{
    SubShader
    {
        // Forward base pass
        Pass
        {
            Tags {"LightMode"="ForwardBase"}
            
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"

            struct v2f
            {
                float4 pos : POSITION;
                float4 shadowCoord : TEXCOORD0;
                float diff : TEXCOORD1;
                fixed2 uv : TEXCOORD2;
            };

            float4x4 _WorldToShadowMatrix;
            float3 _LightDirection;
            
            sampler2D  _ShadowMapTexture;
            SamplerComparisonState sampler_ShadowMapTexture;

            v2f vert (appdata_base v)
            {
                v2f o;

                o.pos = UnityObjectToClipPos(v.vertex);

                o.shadowCoord = mul(_WorldToShadowMatrix, mul( unity_ObjectToWorld, v.vertex ));
                o.shadowCoord = o.shadowCoord/o.shadowCoord.w;
                o.shadowCoord = o.shadowCoord*0.5+0.5;

                float3 worldNormal = UnityObjectToWorldNormal(v.normal);
                o.diff = max(0, dot(worldNormal, _LightDirection.xyz));
                o.uv =  o.shadowCoord.xy;
                return o;
            }

            inline fixed SampleShadow (v2f i)
            {
                float shadow = tex2D(_ShadowMapTexture,i.uv);
                #if defined (UNITY_REVERSED_Z)
                if(shadow > (1-i.shadowCoord.z)){
                    shadow = 0;
                }
                else{
                    shadow = 1;
                }
                #endif
                return shadow;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                fixed4 col = 1;
                fixed shadow = SampleShadow(i);

                return col * shadow * i.diff;
            }
            ENDCG
        }
    }
}
