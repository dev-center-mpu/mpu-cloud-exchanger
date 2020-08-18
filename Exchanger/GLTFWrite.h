#pragma once

#include <conv_i_converter.h> // MbeConvResType

MbeConvResType GLTFWrite(MbModel& model, char*& buffer, size_t& length, char*& thumbnailBuffer, size_t& thumbnailLen, const bool &dracoEncoding, const bool &binaryOutput);