
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

struct DirectionalLight {
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

cbuffer externalData : register(b0) {
	DirectionalLight light;
	DirectionalLight light2;
	float3 CameraPosition;
	float reflectivity;
}

Texture2D diffuseTexture : register(t0);
SamplerState basicSampler : register(s0);
TextureCube Skybox		: register(t2);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	float3 negDir = -light.Direction;
	float3 normDir = normalize(negDir);

	float3 negDir2 = -light2.Direction;
	float3 normDir2 = normalize(negDir2);

	float amt = saturate(dot(input.normal, normDir));
	float amt2 = saturate(dot(input.normal, normDir2));

	float4 res1 = (light.DiffuseColor * amt) + light.AmbientColor;
	float4 res2 = (light2.DiffuseColor * amt2) + light.AmbientColor;

	float4 texColor = diffuseTexture.Sample(basicSampler, input.uv);
	
	float3 toCamera = normalize(CameraPosition - input.position);
	// Sample the skybox
	float4 skyColor = Skybox.Sample(basicSampler, reflect(-toCamera, input.normal));

	float4 litColor = (res1 + res2) * texColor;
	return lerp(litColor, skyColor, reflectivity);
}