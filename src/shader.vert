#version 330 

in vec3 vert;
uniform mat4 mvpMatrix;
uniform mat4 mv;
in vec3 mesh_pos;
in vec2 mesh_text;
in vec3 mesh_normal;

out vec3 m_pos;
out vec2 m_text;
out vec3 m_normal;

out vec3 intensity;
uniform vec4 light; // Light position in eye coords.
uniform vec3 kd; // Diffuse reflectivity
uniform vec3 select;
uniform vec3 ks; 
uniform int shade;

uniform float alpha;
uniform bool DrawSkyBox;
out vec3 box;

out vec3 colour_spec;
uniform mat3 normal;
uniform float shininess;

void main()
{	

	if(DrawSkyBox) {

 		box = -mesh_pos;
 	} 
 	else 
 	{
	    gl_Position = mvpMatrix * vec4(vert, 1.0);

		vec3 tnorm = normalize(normal * mesh_pos);
		vec4 eyeCoords = mvpMatrix * vec4(mesh_pos,1.0);

		vec3 ld = vec3(0.9, 0.9, 0.9);
		vec3 ls = vec3(0, 200, -200);
		vec3 la = vec3(0.5, 0.5, 0.5);
		vec3 ka = vec3(0.1, 0.1, 0.1);

		vec3 s = normalize(vec3(light - eyeCoords));

		vec3 v = normalize(-eyeCoords.xyz);

		//vec3 r = reflect( -s, tnorm );
		
	 	vec3 ambient = ka * la;
		float value = max(dot(s,tnorm), 0.0);
		vec3 diffuse = ld * kd * value;

		vec3 r = -s + 2*(dot(s, tnorm)) * tnorm;
		vec3 spec = vec3(0.0);

		if( value > 0.0 )
		{
			spec = ls * ks * pow(max(dot(r,v), 0.0), shininess * 3);
		}

		intensity = ambient + diffuse;// + spec;

		if( shade == 0)
		{
			intensity = select;
		}
		gl_Position = mvpMatrix * vec4(vert,1.0);

		colour_spec = spec;

		m_text = mesh_text;
		m_normal = normalize(normal * mesh_normal); 
		m_pos = vec3( mv * vec4(mesh_pos,1.0) );
	}
	gl_Position = mvpMatrix * vec4(mesh_pos,1.0);
	
}
