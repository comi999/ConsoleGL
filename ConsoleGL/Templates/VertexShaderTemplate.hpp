// There must be a Layout struct in global scope.
// Its members should all be of a given list of types:
// - mat2, mat3, mat4, vec2, vec3, vec4, float, int, uint,
// Its members names can have prefix tags:
// - uni declares the field as a uniform.
// - in declares the field as an in variable. This means it's a variable being received from an earlier stage.
// - out declares the variable as an out variable. A following stage will use in to receive it.
// - vout declares the variable as a varying out variable. This means following stages will use interpolation to receive it.

struct Layout
{
	mat4 uni_PV;
	mat4 uni_M;
	vec4 in_Position;
	vec4 vout_Position;
	//vec4 out_Position;
};

void run( Layout* layout )
{
	out_Position = uniform_PV * uniform_M * in_Position;
}