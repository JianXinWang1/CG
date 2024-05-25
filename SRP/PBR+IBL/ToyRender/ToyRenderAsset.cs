using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

[CreateAssetMenu(menuName = "Rendering/ToyRenderPipeline")]
public class ToyRenderPipelineAsset : RenderPipelineAsset
{
    public Cubemap diffuse;
    public Cubemap specular;
    public Texture lut;
    protected override RenderPipeline CreatePipeline()
    {
        var Toyrender = new ToyRenderPipeline();
        Toyrender.diffuse = diffuse;
        Toyrender.lut = lut;
        Toyrender.specular = specular;
        return Toyrender;
    }
}