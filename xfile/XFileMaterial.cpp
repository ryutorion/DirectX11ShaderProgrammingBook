#include "XFileMaterial.h"

namespace xfile
{
	bool XFileMaterial::setup(const XFileObject & object)
	{
		if(object.name.compare("Material") != 0)
		{
			return false;
		}

		if(object.dataArray[0].dataType != DataType::Float)
		{
			return false;
		}

		auto & float_list = object.dataArray[0].floatList;
		if(float_list.size() != 11)
		{
			return false;
		}

		faceColor.red = float_list[0];
		faceColor.green = float_list[1];
		faceColor.blue = float_list[2];
		faceColor.alpha = float_list[3];
		power = float_list[4];
		specularColor.red = float_list[5];
		specularColor.green = float_list[6];
		specularColor.blue = float_list[7];
		emissiveColor.red = float_list[8];
		emissiveColor.green = float_list[9];
		emissiveColor.blue = float_list[10];

		if(object.dataArray.size() == 1)
		{
			return true;
		}

		if(object.dataArray[1].dataType != DataType::Object)
		{
			return false;
		}

		if(!textureFilename.setup(*object.dataArray[1].object))
		{
			return false;
		}

		return true;
	}
}
