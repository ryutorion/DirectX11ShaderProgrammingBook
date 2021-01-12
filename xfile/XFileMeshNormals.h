#pragma once
#ifndef XFILE_XFILE_MESH_NORMALS_H_INCLUDED
#define XFILE_XFILE_MESH_NORMALS_H_INCLUDED

#include <cstdint>
#include <vector>
#include "XFileVector.h"
#include "XFileMeshFace.h"
#include "XFileObject.h"

namespace xfile
{
	struct XFileMeshNormals
	{
		bool setup(const XFileObject & object);

		std::vector<XFileVector> normals;
		std::vector<XFileMeshFace> faceNormals;
	};
}

#endif // XFILE_XFILE_MESH_NORMALS_H_INCLUDED
