#ifndef RENDER_H_GUARD
#define RENDER_H_GUARD

#include <texture.h>
#include <shader.h>
#include <vao.h>
#include <ubo.h>
#include "hitbox.h"
#include "types.h"

#include <vector>
#include <unordered_map>
#include <glm/mat4x4.hpp>

class Render
{
private:
	glm::mat4 projection;

	Ubo uniforms;
	Vao vSprite;
	Vao vGeometry;
	enum{
		LINES = 0,
		BOXES,
		GEO_SIZE
	};
	int geoParts[GEO_SIZE];
	std::vector<float> clientQuads;
	std::vector<uint16_t> clientElements;
	int quadsToDraw;
	size_t acumSize = 0;
	size_t acumQuads = 0;
	size_t acumElements = 0;
	float zOrder = 0;

	int lAlphaS;
	Shader sSimple;
	Shader sTextured;
	Texture texture;
	float colorRgba[4];
	
	void SetModelView(glm::mat4&& view);
	int nSprites;

public:
	enum color_t 
	{
		gray = 0,
		green,
		red,
	};

	int spriteId;
	int x, offsetX;
	int y, offsetY;
	float scale;
	float scaleX, scaleY;
	float rotX, rotY, rotZ;
	int highLightN = -1;
	
	Render();
	NameMap gfxNames;
	void LoadGraphics(const char* pngFile, const char* vtxFile);
	void LoadPalette(const char* file);

	void Draw();
	void UpdateProj(float w, float h);

	void GenerateHitboxVertices(const std::vector<int> &boxes, color_t pickedColor);
	void LoadHitboxVertices();

	void DontDraw();
	void SetImageColor(float *rgbaArr);
};

#endif /* RENDER_H_GUARD */
