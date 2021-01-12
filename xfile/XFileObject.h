#pragma once
#ifndef XFILE_XFILE_OBJECT_H_INCLUDED
#define XFILE_XFILE_OBJECT_H_INCLUDED

#include <vector>
#include <string>
#include "XFileData.h"

namespace xfile
{
	struct XFileObject
	{
		std::string name;
		std::string optionalName;
		std::vector<XFileData> dataArray;
	};
}

#endif // XFILE_XFILE_OBJECT_H_INCLUDED
