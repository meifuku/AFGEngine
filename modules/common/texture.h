#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED
#include <string>
#include <image.h>
#include <memory>

extern int pal;

class Texture //Point to it, do not use directly.
{
public: //public only for ease of reading access, do not write anything.
	std::unique_ptr<ImageData> image;
	unsigned int id; //OpenGL texture id.
	bool isLoaded;
	bool isApplied;
	std::string filename;
	

public:
	Texture();
	Texture(Texture&& texture);
	~Texture();

	//Set doNotFixAlphaGamma to true to NOT modify the image when loading it.
	void Load(std::string imageFile, std::string paletteFile = std::string(), bool doNotFixAlphaGamma = false);
	void Apply(bool repeat = false, bool linearFilter = false);
	void Unapply();
	void Unload();

};

#endif // IMAGE_H_INCLUDED
