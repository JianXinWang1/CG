using UnityEngine;
using UnityEngine.Rendering;


public class SimpleRenderPipeline : RenderPipeline
{
    static readonly ShaderTagId s_OpaquePassTag = new ShaderTagId("ForwardBase"); 

    RenderTexture m_ShadowMap;

    protected override void Render(ScriptableRenderContext context, Camera[] cameras)
    {

        var camera = cameras[0];
        if (m_ShadowMap == null)
        {
            m_ShadowMap = new RenderTexture(1024, 1024, 24, RenderTextureFormat.Shadowmap);
            m_ShadowMap.name = "Shadow Map";
        }


        camera.TryGetCullingParameters(out ScriptableCullingParameters cullingParameters);
       

        cullingParameters.shadowDistance = 30;

        // 裁剪
        CullingResults cullingResults = context.Cull(ref cullingParameters);

        // 深度纹理生成
        Matrix4x4 worldToShadowMatrix = Matrix4x4.identity;
        bool didRenderShadowMap = RenderShadowMaps(ref context, ref cullingResults, ref worldToShadowMatrix);

        // 设置
        context.SetupCameraProperties(camera);

        CommandBuffer cb_ClearCamera = new CommandBuffer();
        cb_ClearCamera.name = "ClearCamera";
        camera.targetTexture = null;
        cb_ClearCamera.SetRenderTarget(camera.targetTexture); 
        cb_ClearCamera.ClearRenderTarget(true, true, camera.backgroundColor);
        context.ExecuteCommandBuffer(cb_ClearCamera);

        // 绘制物体
        SortingSettings sortSettings = new SortingSettings(camera);
        sortSettings.criteria = SortingCriteria.CommonOpaque;

        FilteringSettings filterSettings = FilteringSettings.defaultValue;
        filterSettings.renderQueueRange = RenderQueueRange.opaque;

        
        CommandBuffer cb = new CommandBuffer();
        cb.name = "Set up shadow shader properties";
        cb.SetGlobalTexture("_ShadowMapTexture", m_ShadowMap);
        cb.SetGlobalMatrix("_WorldToShadowMatrix", worldToShadowMatrix);
        cb.SetGlobalVector("_LightDirection", -cullingResults.visibleLights[0].light.transform.forward); // Direction towards the light
        context.ExecuteCommandBuffer(cb);
        

        DrawingSettings opaqueDrawSettings = new DrawingSettings(s_OpaquePassTag, sortSettings);
        context.DrawRenderers(cullingResults, ref opaqueDrawSettings, ref filterSettings);

        // 绘制背景
        context.DrawSkybox(camera);
        context.Submit();

        
        

    }
    
    private bool RenderShadowMaps(ref ScriptableRenderContext context, ref CullingResults cullingResults, ref Matrix4x4 worldToShadowMatrix)
    {


        int lightIndex = 0; 

        bool needToRender = cullingResults.ComputeDirectionalShadowMatricesAndCullingPrimitives(lightIndex, 0, 1, Vector3.forward, 1024, cullingResults.visibleLights[0].light.shadowNearPlane, out Matrix4x4 viewMatrix, out Matrix4x4 projMatrix, out ShadowSplitData shadowSplitData);

        if (!needToRender)
            return false;

        CommandBuffer cb = new CommandBuffer();
        cb.name = "Set up shadow data";
        cb.SetRenderTarget(m_ShadowMap);
        cb.ClearRenderTarget(true, true, Color.clear);
        cb.SetViewProjectionMatrices(viewMatrix, projMatrix);
        context.ExecuteCommandBuffer(cb);

        ShadowDrawingSettings shadowDrawSettings = new ShadowDrawingSettings(cullingResults, lightIndex);

        
        context.DrawShadows(ref shadowDrawSettings);




        worldToShadowMatrix =
			 (projMatrix * viewMatrix);

        return true;
    }

}
