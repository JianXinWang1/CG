using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEditor;

public class ToyRenderPipeline : RenderPipeline
{
    RenderTexture gdepth;                                               // depth 
    RenderTexture[] gbuffers = new RenderTexture[5];                    // color 
    RenderTargetIdentifier[] gbufferID = new RenderTargetIdentifier[5]; // tex 
    public Cubemap diffuse;
    public Cubemap specular;
    public Texture lut;
    public ToyRenderPipeline()
    {
        // 创建纹理
        gdepth = new RenderTexture(Screen.width, Screen.height, 24, RenderTextureFormat.Depth, RenderTextureReadWrite.Linear);
        gbuffers[0] = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB32, RenderTextureReadWrite.Linear);
        gbuffers[1] = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB2101010, RenderTextureReadWrite.Linear);
        gbuffers[2] = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB64, RenderTextureReadWrite.Linear);
        gbuffers[3] = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Linear);
        gbuffers[4] = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Linear);

        // 给纹理 ID 赋值
        for (int i = 0; i < 5; i++)
            gbufferID[i] = gbuffers[i];
    }
    void LightPass(ScriptableRenderContext context, Camera camera)
    {

        // 使用 Blit
        CommandBuffer cmd = new CommandBuffer();

        cmd.SetGlobalTexture("_gdepth", gdepth);
        // 设置相机矩阵
        Matrix4x4 viewMatrix = camera.worldToCameraMatrix;
        Matrix4x4 projMatrix = GL.GetGPUProjectionMatrix(camera.projectionMatrix, false);
        Matrix4x4 vpMatrix = projMatrix * viewMatrix;
        Matrix4x4 vpMatrixInv = vpMatrix.inverse;
        cmd.SetGlobalMatrix("_vpMatrix", vpMatrix);
        cmd.SetGlobalMatrix("_vpMatrixInv", vpMatrixInv);
        cmd.SetGlobalTexture("_diffuse", diffuse);
        cmd.SetGlobalTexture("_specular",specular );
        cmd.SetGlobalTexture("_lut", lut);
        for (int i = 0; i < 5; i++)
            cmd.SetGlobalTexture("_GT" + i, gbuffers[i]);
        cmd.name = "lightpass";
        Material mat = new Material(Shader.Find("ToyRP/lightpass"));
        cmd.Blit(gbufferID[0], BuiltinRenderTextureType.CameraTarget, mat);
        context.ExecuteCommandBuffer(cmd);

    }




    protected override void Render(ScriptableRenderContext context, Camera[] cameras)
    {
        // 主相机
        Camera camera = cameras[0];
        context.SetupCameraProperties(camera);

        CommandBuffer cmd = new CommandBuffer();
        cmd.name = "gbuffer";

        // 设置 gbuffer 为全局纹理

        // 清屏
        cmd.SetRenderTarget(gbufferID, gdepth);
        cmd.ClearRenderTarget(true, true, Color.red);
        context.ExecuteCommandBuffer(cmd);

        // 剔除
        camera.TryGetCullingParameters(out var cullingParameters);
        var cullingResults = context.Cull(ref cullingParameters);

        // config settings
        ShaderTagId shaderTagId = new ShaderTagId("gbuffer");   // 使用 LightMode 为 gbuffer 的 shader
        SortingSettings sortingSettings = new SortingSettings(camera);
        DrawingSettings drawingSettings = new DrawingSettings(shaderTagId, sortingSettings);
        FilteringSettings filteringSettings = FilteringSettings.defaultValue;

        // 绘制
        context.DrawRenderers(cullingResults, ref drawingSettings, ref filteringSettings);



        
        LightPass(context, camera);
        context.DrawSkybox(camera);
        context.Submit();



    }
}