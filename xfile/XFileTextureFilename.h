#pragma once
#ifndef XFILE_XFILE_TEXTURE_FILE_NAME_H_INCLUDED
#define XFILE_XFILE_TEXTURE_FILE_NAME_H_INCLUDED

#include <string>
#include "XFileObject.h"

namespace xfile
{
	struct XFileTextureFilename
	{
		bool setup(const XFileObject & object);

		std::string filename;
	};
}

#endif // XFILE_XFILE_TEXTURE_FILE_NAME_H_INCLUDED
