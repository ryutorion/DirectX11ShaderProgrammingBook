#include "XFileMeshTextureCoords.h"

namespace xfile
{
	bool XFileMeshTextureCoords::setup(const XFileObject & object)
	{
		if(object.name.compare("MeshTextureCoords") != 0)
		{
			return false;
		}

		if(object.dataArray[0].dataType != DataType::Integer)
		{
			return false;
		}

		auto texture_coord_count = object.dataArray[0].numberList[0];
		textureCoords.resize(texture_coord_count);

		memcpy(
			textureCoords.data(),
			object.dataArray[1].floatList.data(),
			sizeof(XFileCoords2d) * texture_coord_count
		);

		return true;
	}
}
