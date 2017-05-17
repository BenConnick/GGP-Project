struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexToPixel main(uint id : SV_VertexID)
{
	// high level explanation:
	/*
	The vertex shader is passed three verticies
	and creates a triangle that covers the whole screen
	With uvs that go from 0 to 1 within the bounds of the screen
	 ___________________
	|			|	  /
	|	screen	|	/
	|			| /          
	 ------------
	|          /
	|        /
	|     /
	|  /
	|/
	*/

	// output using struct
	VertexToPixel output;

	// use id to come up with uv coords from (0,0) to (2,2)
	output.uv = float2(
		(id << 1) & 2, // bitshifting to achieve id % 2 * 2 for x coord
		id & 2);

	// make positions based on the UV 
	// since we are only outputting the screen points
	output.position = float4(output.uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;

	return output;
}