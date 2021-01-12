#pragma once
#ifndef XFILE_XFILE_MESH_FACE_H_INCLUDED
#define XFILE_XFILE_MESH_FACE_H_INCLUDED

#include <cstdint>
#include <vector>

namespace xfile
{
	struct XFileMeshFace
	{
		std::vector<uint32_t> faceVertexIndices;
	};
}

#endif // XFILE_XFILE_MESH_FACE_H_INCLUDED
