#include "XFileReader.h"
#include <utility>
#include "XFileMesh.h"

namespace
{
	constexpr uint32_t XFileMagic = 'x' | ('o' << 8) | ('f' << 16) | (' ' << 24);
	constexpr uint32_t XFileFormatBinary = 'b' | ('i' << 8) | ('n' << 16) | (' ' << 24);
	constexpr uint32_t XFileFormatText = 't' | ('x' << 8) | ('t' << 16) | (' ' << 24);
	constexpr uint32_t XFileFormatCompressed = 'c' | ('m' << 8) | ('p' << 16) | (' ' << 24);
	constexpr uint32_t XFileFormatFloatBits32 = '0' | ('0' << 8) | ('3' << 16) | ('2' << 24);
	constexpr uint32_t XFileFormatFloatBits64 = '0' | ('0' << 8) | ('6' << 16) | ('4' << 24);

	std::istream & operator>>(std::istream & in, xfile::TokenType & token)
	{
		if(in.eof())
		{
			token = xfile::TokenType::None;
			return in;
		}

		int16_t t;
		in.read(reinterpret_cast<char *>(&t), sizeof(t));

		switch(static_cast<xfile::TokenType>(t))
		{
		case xfile::TokenType::Name:
		case xfile::TokenType::String:
		case xfile::TokenType::Integer:
		case xfile::TokenType::GUID:
		case xfile::TokenType::IntegerList:
		case xfile::TokenType::FloatList:
		case xfile::TokenType::OpenBrace:
		case xfile::TokenType::CloseBrace:
		case xfile::TokenType::OpenParen:
		case xfile::TokenType::CloseParen:
		case xfile::TokenType::OpenBracket:
		case xfile::TokenType::CloseBracket:
		case xfile::TokenType::OpenAngle:
		case xfile::TokenType::CloseAngle:
		case xfile::TokenType::Dot:
		case xfile::TokenType::Comma:
		case xfile::TokenType::SemiColon:
		case xfile::TokenType::Template:
		case xfile::TokenType::Word:
		case xfile::TokenType::DoubleWord:
		case xfile::TokenType::Float:
		case xfile::TokenType::Double:
		case xfile::TokenType::Char:
		case xfile::TokenType::UnsignedChar:
		case xfile::TokenType::SignedWord:
		case xfile::TokenType::SignedDoubleWord:
		case xfile::TokenType::Void:
		case xfile::TokenType::StringPointer:
		case xfile::TokenType::Unicode:
		case xfile::TokenType::CString:
		case xfile::TokenType::Array:
			token = static_cast<xfile::TokenType>(t);
			break;
		default:
			if(in.eof()) 
			{
				token = xfile::TokenType::None;
			}
			else
			{
				token = xfile::TokenType::Error;
			}
			break;
		}

		return in;
	}
}

namespace xfile
{
	XFileReader::XFileReader(const char * p_file_path)
	{
		open(p_file_path);
	}

	XFileReader::~XFileReader()
	{
		close();
	}

	bool XFileReader::open(const char * p_file_path)
	{
		if(mFin.is_open())
		{
			return false;
		}

		mFin.open(p_file_path, std::ios::in | std::ios::binary);
		if(!mFin)
		{
			return false;
		}

		uint32_t magic;
		mFin.read(reinterpret_cast<char *>(&magic), sizeof(magic));
		if(magic != XFileMagic)
		{
			return false;
		}

		// ドキュメントではバージョンは0302となっているが，
		// DirectX 9シェーダプログラミングブックのバージョンは0303のため
		// いったんバージョンは無視する
		uint32_t version;
		mFin.read(reinterpret_cast<char *>(&version), sizeof(version));

		uint32_t format;
		mFin.read(reinterpret_cast<char *>(&format), sizeof(format));
		switch(format)
		{
		case XFileFormatBinary:
			mFormat = Format::Binary;
			break;
		case XFileFormatText:
			mFormat = Format::Text;
			break;
		case XFileFormatCompressed:
			mFormat = Format::Compressed;
			break;
		default:
			return false;
		}

		uint32_t float_format;
		mFin.read(reinterpret_cast<char *>(&float_format), sizeof(float_format));
		switch(float_format)
		{
		case XFileFormatFloatBits32:
			mFloatFormat = FloatFormat::Bits32;
			break;
		case XFileFormatFloatBits64:
			mFloatFormat = FloatFormat::Bits64;
			break;
		default:
			return false;
		}

		return true;
	}

	bool XFileReader::read(XFile & xfile)
	{

		if(!readNextTokenType())
		{
			return false;
		}

		while(true)
		{
			XFileObject object;
			if(!readObject(object))
			{
				break;
			}

			if(object.name.compare("Mesh") == 0)
			{
				XFileMesh mesh;
				if(!mesh.setup(object))
				{
					return false;
				}

				xfile.meshes.emplace_back(std::move(mesh));
			}

			if(!readNextTokenType())
			{
				return false;
			}

			if(mNextTokenType == TokenType::None)
			{
				break;
			}
		}

		return true;
	}

	bool XFileReader::close()
	{
		if(mFin.is_open())
		{
			mFin.close();
		}

		return true;
	}

	bool XFileReader::readObject(XFileObject & object)
	{
		if(mNextTokenType == TokenType::Name)
		{
			if(!readName(object.name))
			{
				return false;
			}

			if(!readNextTokenType())
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		if(mNextTokenType == TokenType::Name)
		{
			if(!readName(object.optionalName))
			{
				return false;
			}

			if(!readNextTokenType())
			{
				return false;
			}
		}

		if(mNextTokenType != TokenType::OpenBrace)
		{
			return false;
		}

		if(!readNextTokenType())
		{
			return false;
		}

		while(mNextTokenType != TokenType::CloseBrace)
		{
			XFileData data;
			switch(mNextTokenType)
			{
			case TokenType::Name:
				data.dataType = DataType::Object;
				data.object.reset(new XFileObject);
				if(!readObject(*data.object))
				{
					return false;
				}
				break;
			case TokenType::IntegerList:
				if(!readIntegerList(data))
				{
					return false;
				}
				break;
			case TokenType::FloatList:
				if(!readFloatList(data))
				{
					return false;
				}
				break;
			case TokenType::String:
				if(!readString(data))
				{
					return false;
				}
				break;
			default:
				return false;
			}

			object.dataArray.emplace_back(std::move(data));

			if(!readNextTokenType())
			{
				return false;
			}
		}

		return true;
	}

	bool XFileReader::readNextTokenType()
	{
		mFin >> mNextTokenType;
		if(mNextTokenType == TokenType::Error)
		{
			return false;
		}

		return true;
	}

	bool XFileReader::readName(std::string & s)
	{
		uint32_t length;
		mFin.read(reinterpret_cast<char *>(&length), sizeof(length));

		s.resize(length);
		mFin.read(s.data(), length);

		return true;
	}

	bool XFileReader::readGUID()
	{
		uint32_t data1;
		mFin.read(reinterpret_cast<char *>(&data1), sizeof(data1));

		uint16_t data2;
		mFin.read(reinterpret_cast<char *>(&data2), sizeof(data2));

		uint16_t data3;
		mFin.read(reinterpret_cast<char *>(&data3), sizeof(data3));

		uint8_t data4[8];
		mFin.read(reinterpret_cast<char *>(data4), sizeof(data4));

		return true;
	}

	bool XFileReader::readIntegerList(XFileData & data)
	{
		uint32_t count;
		mFin.read(reinterpret_cast<char *>(&count), sizeof(count));

		data.dataType = DataType::Integer;
		data.numberList.resize(count);
		mFin.read(reinterpret_cast<char *>(data.numberList.data()), sizeof(uint32_t) * count);

		return true;
	}

	bool XFileReader::readFloatList(XFileData & data)
	{
		uint32_t count;
		mFin.read(reinterpret_cast<char *>(&count), sizeof(count));

		if(mFloatFormat == FloatFormat::Bits32)
		{
			data.dataType = DataType::Float;
			data.floatList.resize(count);
			mFin.read(reinterpret_cast<char *>(data.floatList.data()), sizeof(float) * count);
		}
		else if(mFloatFormat == FloatFormat::Bits64)
		{
			data.dataType = DataType::Double;
			data.doubleList.resize(count);
			mFin.read(reinterpret_cast<char *>(data.doubleList.data()), sizeof(double) * count);
		}
		else
		{
			return false;
		}

		return true;
	}
	bool XFileReader::readString(XFileData & data)
	{
		uint32_t count;
		mFin.read(reinterpret_cast<char *>(&count), sizeof(count));

		std::string s(count, '\0');
		mFin.read(reinterpret_cast<char *>(s.data()), count);

		if(!readNextTokenType())
		{
			return false;
		}

		if(mNextTokenType != TokenType::Comma && mNextTokenType != TokenType::SemiColon)
		{
			return false;
		}

		data.dataType = DataType::String;
		data.stringList.emplace_back(std::move(s));

		return true;
	}
}
