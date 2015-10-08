#version 330

uniform vec3 frag_color;
uniform sampler2D text_colour;
out vec4 finalColor;

in vec3 intensity;
in vec2 m_text;
in vec3 colour_spec;
in vec3 m_vert;

uniform samplerCube CubeMapTex; // The cube map

uniform float alpha;
uniform bool DrawSkyBox; // Are we drawing the sky box?
in vec3 box;

void main()
{
	if(DrawSkyBox)
	{
		vec4 cubeMapColor = texture(CubeMapTex,box);
		finalColor = cubeMapColor;
	}
	else
	{
	    vec4 textureColour = texture(text_colour, m_text);
	    finalColor = vec4(intensity, 1.0) * textureColour; // + vec4(colour_spec, 1.0);
	    finalColor.w = alpha;
	}

}
