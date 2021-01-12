#include "XFileMesh.h"

namespace xfile
{
	bool XFileMesh::setup(const XFileObject & object)
	{
		if(object.name.compare("Mesh") != 0)
		{
			return false;
		}

		name = object.optionalName;

		if(object.dataArray[0].dataType != DataType::Integer)
		{
			return false;
		}

		if(object.dataArray[0].numberList.size() != 1)
		{
			return false;
		}

		auto vertex_count = object.dataArray[0].numberList[0];
		vertices.resize(vertex_count);

		if(object.dataArray[1].dataType != DataType::Float)
		{
			return false;
		}

		if(object.dataArray[1].floatList.size() != vertex_count * 3)
		{
			return false;
		}

		memcpy(
			vertices.data(),
			object.dataArray[1].floatList.data(),
			sizeof(object.dataArray[1].floatList[0]) * object.dataArray[1].floatList.size()
		); 
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

			faces.emplace_back(std::move(mesh_face));

			i += 4;
		}

		for(size_t i = 3; i < object.dataArray.size(); ++i)
		{
			if(object.dataArray[i].dataType != DataType::Object)
			{
				return false;
			}

			if(object.dataArray[i].object->name.compare("MeshNormals") == 0)
			{
				if(!normals.setup(*object.dataArray[i].object))
				{
					return false;
				}
			}
			else if(object.dataArray[i].object->name.compare("MeshTextureCoords") == 0)
			{
				if(!textureCoords.setup(*object.dataArray[i].object))
				{
					return false;
				}
			}
			else if(object.dataArray[i].object->name.compare("MeshMaterialList") == 0)
			{
				if(!materialList.setup(*object.dataArray[i].object))
				{
					return false;
				}
			}
		}

		return true;
	}
}
