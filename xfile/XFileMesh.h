#pragma once
#ifndef XFILE_XFILE_MESH_H_INCLUDED
#define XFILE_XFILE_MESH_H_INCLUDED

#include <cstdint>
#include <string>
#include <vector>
#include "XFileObject.h"
#include "XFileVector.h"
#include "XFileMeshFace.h"
#include "XFileMeshNormals.h"
#include "XFileMeshTextureCoords.h"
#include "XFileMeshMaterialList.h"

namespace xfile
{
	struct XFileObject;

	struct XFileMesh
	{
		bool setup(const XFileObject & object);

		std::string name;
		std::vector<XFileVector> vertices;
		std::vector<XFileMeshFace> faces;
		XFileMeshNormals normals;
		XFileMeshTextureCoords textureCoords;
		XFileMeshMaterialList materialList;
	};
}

#endif // XFILE_XFILE_MESH_H_INCLUDED
