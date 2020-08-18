#pragma once
#include <string>

namespace mpu {
	bool Translate(char*& inFileBuffer, size_t inFileLen, std::string inFileExt, 
		char*& outFileBuffer, size_t& outFileLen, std::string outFileExt, 
		char*& thumbnailBuffer, size_t& thumbnailLen,
		std::string &errorMessage);
}