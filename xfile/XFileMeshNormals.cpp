#include "XFileMeshNormals.h"

namespace xfile
{
	bool XFileMeshNormals::setup(const XFileObject & object)
	{
		if(object.name.compare("MeshNormals") != 0)
		{
			return false;
		}

		if(object.dataArray[0].dataType != DataType::Integer)
		{
			return false;
		}

		if(object.dataArray[0].numberList.size() != 1)
		{
			return false;
		}

		auto normal_count = object.dataArray[0].numberList[0];
		normals.resize(normal_count);

		if(object.dataArray[1].dataType != DataType::Float)
		{
			return false;
		}

		if(object.dataArray[1].floatList.size() != normal_count * 3)
		{
			return false;
		}

		memcpy(normals.data(), object.dataArray[1].floatList.data(), sizeof(float) * normal_count);

		if(object.dataArray[2].dataType != DataType::Integer)
		{
			return false;
		}

		auto & f = object.dataArray[2].numberList;
		[[maybe_unused]] auto face_count = f[0];

		for(size_t i = 1; i < f.size();)
		{
			auto face_index_count = f[i];
			if(face_index_count != 3)
			{
				return false;
			}

			XFileMeshFace mesh_face
			{
				.faceVertexIndices = { f[i + 1], f[i + 2], f[i + 3] }
			};

			faceNormals.emplace_back(std::move(mesh_face));

			i += 4;
		}


		return true;
	}
}
