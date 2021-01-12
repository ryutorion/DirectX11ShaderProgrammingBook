#pragma once
#ifndef XFILE_XFILE_MESH_TEXTURE_COORDS_H_INCLUDED
#define XFILE_XFILE_MESH_TEXTURE_COORDS_H_INCLUDED

#include <vector>
#include "XFileCoords2d.h"
#include "XFileObject.h"

namespace xfile
{
	struct XFileMeshTextureCoords
	{
		bool setup(const XFileObject & object);

		std::vector<XFileCoords2d> textureCoords;
	};
}

#endif // XFILE_XFILE_MESH_TEXTURE_COORDS_H_INCLUDED
