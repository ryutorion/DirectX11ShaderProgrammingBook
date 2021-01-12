#pragma once
#ifndef XFILE_XFILE_H_INCLUDED
#define XFILE_XFILE_H_INCLUDED

#include <vector>
#include "XFileMesh.h"

namespace xfile
{
	struct XFile
	{
		std::vector<XFileMesh> meshes;
	};
}

#endif // XFILE_XFILE_H_INCLUDED
