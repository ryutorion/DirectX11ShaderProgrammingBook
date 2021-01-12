#pragma once
#ifndef XFILE_XFILE_DATA_H_INCLUDED
#define XFILE_XFILE_DATA_H_INCLUDED

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace xfile
{
	struct XFileObject;

	enum class DataType
	{
		Integer,
		Float,
		Double,
		String,
		Object,
	};

	struct XFileData
	{
		DataType dataType;
		std::vector<uint32_t> numberList;
		std::vector<float> floatList;
		std::vector<double> doubleList;
		std::vector<std::string> stringList;
		std::unique_ptr<XFileObject> object;
	};
}

#endif // XFILE_XFILE_DATA_H_INCLUDED
