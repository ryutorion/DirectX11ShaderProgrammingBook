#include "XFileMeshMaterialList.h"

namespace xfile
{
	bool XFileMeshMaterialList::setup(const XFileObject & object)
	{
		if(object.name.compare("MeshMaterialList") != 0)
		{
			return false;
		}

		if(object.dataArray[0].dataType != DataType::Integer)
		{
			return false;
		}

		[[maybe_unused]]
		auto material_count = object.dataArray[0].numberList[0];

		auto face_index_count = object.dataArray[0].numberList[1];
		faceIndexes.resize(face_index_count);
		memcpy(
			faceIndexes.data(),
			&object.dataArray[0].numberList[2],
			sizeof(uint32_t) * face_index_count
		);

		for(size_t i = 1; i < object.dataArray.size(); ++i)
		{
			if(object.dataArray[i].dataType != DataType::Object)
			{
				return false;
			}

			XFileMaterial material;
			if(!material.setup(*object.dataArray[i].object))
			{
				return false;
			}

			materials.emplace_back(std::move(material));
		}

		return true;
	}
}
