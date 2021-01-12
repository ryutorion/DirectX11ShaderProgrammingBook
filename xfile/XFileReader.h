#pragma once
#ifndef XFILE_XFILE_READER_H_INCLUDED
#define XFILE_XFILE_READER_H_INCLUDED

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "XFileObject.h"
#include "XFile.h"

namespace xfile {
	enum class TokenType
	{
		Error = -1,
		None = 0,
		Name = 1,
		String = 2,
		Integer = 3,
		GUID = 5,
		IntegerList = 6,
		FloatList = 7,
		OpenBrace = 10,
		CloseBrace = 11,
		OpenParen = 12,
		CloseParen = 13,
		OpenBracket = 14,
		CloseBracket = 15,
		OpenAngle = 16,
		CloseAngle = 17,
		Dot = 18,
		Comma = 19,
		SemiColon = 20,
		Template = 31,
		Word = 40,
		DoubleWord = 41,
		Float = 42,
		Double = 43,
		Char = 44,
		UnsignedChar = 45,
		SignedWord = 46,
		SignedDoubleWord = 47,
		Void = 48,
		StringPointer = 49,
		Unicode = 50,
		CString = 51,
		Array = 52
	};

	class XFileReader
	{
	public:
		XFileReader() = default;
		XFileReader(const char * p_file_path);

		~XFileReader();

		bool open(const char * p_file_path);
		bool read(XFile & xfile);
		bool close();
	private:
		enum class Format
		{
			Unknown,
			Binary,
			Text,
			Compressed,
		};

		enum class FloatFormat
		{
			Unknown,
			Bits32,
			Bits64,
		};

	private:
		bool readObject(XFileObject & object);
		bool readNextTokenType();
		bool readName(std::string & s);
		bool readGUID();
		bool readIntegerList(XFileData & data);
		bool readFloatList(XFileData & data);
		bool readString(XFileData & data);

	private:
		std::ifstream mFin;
		Format mFormat = Format::Unknown;
		FloatFormat mFloatFormat = FloatFormat::Unknown;
		TokenType mNextTokenType = TokenType::None;

		std::vector<XFileObject> mObjects;
	};
}

#endif // XFILE_XFILE_READER_H_INCLUDED
