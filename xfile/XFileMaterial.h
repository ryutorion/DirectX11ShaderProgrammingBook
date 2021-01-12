#pragma once
#ifndef XFILE_XFILE_MATERIAL_H_INCLUDED
#define XFILE_XFILE_MATERIAL_H_INCLUDED

#include "XFileObject.h"
#include "XFileColorRGBA.h"
#include "XFileColorRGB.h"
#include "XFileTextureFilename.h"

namespace xfile
{
	struct XFileMaterial
	{
		bool setup(const XFileObject & object);

		XFileColorRGBA faceColor;
		float power;
		XFileColorRGB specularColor;
		XFileColorRGB emissiveColor;
		XFileTextureFilename textureFilename;
	};
}

#endif // XFILE_XFILE_MATERIAL_H_INCLUDED
