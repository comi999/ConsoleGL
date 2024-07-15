// There must be a Layout struct in global scope.
// Its members should all be of a given list of types:
// - mat2, mat3, mat4, vec2, vec3, vec4, float, int, uint,
// Its members names can have prefix tags:
// - uni declares the field as a uniform.
// - in declares the field as an in variable. This means it's a variable being received from an earlier stage.
// - out declares the variable as an out variable. A following stage will use in to receive it.
// - vin declares the variable as a varying in variable. This means the previous stage will use interpolation on it.
// - vout declares the variable as a varying out variable. This means following stages will use interpolation to receive it.

struct Layout
{
	mat4 uni_PV;
	mat4 uni_M;
	vec4 vin_Position;
	vec4 out_Colour;
};

void run( Layout* layout )
{
	// ... Do some shader stuff
	o_Fragment->r = 1.0f;
	o_Fragment->g = 0.4f;
	o_Fragment->b = 0.2f;
	o_Fragment->a = 1.0f;
}