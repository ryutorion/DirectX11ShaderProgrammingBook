#pragma once
#ifndef XFILE_XFILE_MESH_MATERIAL_LIST_H_INCLUDED
#define XFILE_XFILE_MESH_MATERIAL_LIST_H_INCLUDED

#include <vector>
#include "XFileObject.h"
#include "XFileMaterial.h"

namespace xfile
{
	struct XFileMeshMaterialList
	{
		bool setup(const XFileObject & object);

		std::vector<uint32_t> faceIndexes;
		std::vector<XFileMaterial> materials;
	};
}

#endif // XFILE_XFILE_MESH_MATERIAL_LIST_H_INCLUDED
