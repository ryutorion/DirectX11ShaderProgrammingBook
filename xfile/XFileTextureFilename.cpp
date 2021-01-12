#include "XFileTextureFilename.h"

namespace xfile
{
	bool XFileTextureFilename::setup(const XFileObject & object)
	{
		if(object.name.compare("TextureFilename") != 0)
		{
			return false;
		}

		if(object.dataArray[0].dataType != DataType::String)
		{
			return false;
		}

		filename = object.dataArray[0].stringList[0];

		return true;
	}
}
