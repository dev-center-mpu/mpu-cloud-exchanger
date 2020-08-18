#include "Translate.h"

#include <model.h> // MbModel
#include <conv_i_converter.h> // ImportFromBuffer()
#include <conv_model_properties.h> // ConvConvertorProperty3D

#include "GLTFWrite.h"

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

namespace mpu {
	bool Translate(char*& inFileBuffer, size_t inFileLen, std::string inFileExt,
		char*& outFileBuffer, size_t& outFileLen, std::string outFileExt,
		char*& thumbnailBuffer, size_t& thumbnailLen,
		std::string& errorMessage)
	{
		ConvConvertorProperty3D props;
		props.enableAutostitch = true;
		props.replaceLocationsToRight = true;
		props.joinSimilarFaces = true;
		props.dualSeams = true;
		props.addRemovedFacesAsShells = true;

		std::transform(inFileExt.begin(), inFileExt.end(), inFileExt.begin(), [](unsigned char c) { return std::tolower(c); });
		std::transform(outFileExt.begin(), outFileExt.end(), outFileExt.begin(), [](unsigned char c) { return std::tolower(c); });

		MbeConvResType importResult;
		MbModel model;

		switch (str2int(inFileExt.c_str())) {
		case str2int("stp"): case str2int("step"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_STEP, &props); break;
		case str2int("igs"): case str2int("iges"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_IGES, &props); break;
		case str2int("sat"): case str2int("acis"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_ACIS, &props); break;
		case str2int("jt"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_JT, &props); break;
		case str2int("x_t"): case str2int("x_b"): case str2int("xmt_txt"): case str2int("xmp_txt"): case str2int("xmt_bin"): case str2int("xmp_bin"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_Parasolid, &props); break;
		case str2int("c3d"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_C3D, &props); break;
		case str2int("wrl"): case str2int("vrml"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_VRML, &props); break;
		case str2int("stl"):
			importResult = c3d::ImportFromBuffer(model, inFileBuffer, inFileLen, mxf_STL, &props); break;
		default: importResult = cnv_UnknownExtension;
		}

		if (importResult != cnv_Success) {
			switch (importResult)
			{
			case cnv_NoBody:  errorMessage = "Bodies hasn't been found"; break;
			case cnv_NoObjects: errorMessage = "Objects hasn't been found"; break;
			case cnv_FileOpenError: errorMessage = "Cannot open file"; break;
			case cnv_ImpossibleReadAssembly: errorMessage = "Cannot read assembly"; break;
			case cnv_LicenseNotFound:  errorMessage = "Core license expired"; break;
			case cnv_UnknownExtension: errorMessage = "Unknown extention"; break;
			case cnv_NotEnoughMemory:  errorMessage = "Not enough memory"; break;
			case cnv_Error: errorMessage = "Unknown import error"; break;
			default: errorMessage = "Unknown error"; break;
			}

			return false;
		}

		MbeConvResType exportResult;

		switch (str2int(outFileExt.c_str())) {
		case str2int("stp"): case str2int("step"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_STEP, outFileBuffer, outFileLen, &props); break;
		case str2int("igs"): case str2int("iges"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_IGES, outFileBuffer, outFileLen, &props); break;
		case str2int("sat"): case str2int("acis"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_ACIS, outFileBuffer, outFileLen, &props); break;
		case str2int("jt"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_JT, outFileBuffer, outFileLen, &props); break;
		case str2int("x_t"): case str2int("x_b"): case str2int("xmt_txt"): case str2int("xmp_txt"): case str2int("xmt_bin"): case str2int("xmp_bin"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_Parasolid, outFileBuffer, outFileLen, &props); break;
		case str2int("c3d"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_C3D, outFileBuffer, outFileLen, &props); break;
		case str2int("wrl"): case str2int("vrml"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_VRML, outFileBuffer, outFileLen, &props); break;
		case str2int("stl"):
			exportResult = c3d::ExportIntoBuffer(model, mxf_STL, outFileBuffer, outFileLen, &props); break;
		case str2int("gltf"):
			exportResult = GLTFWrite(model, outFileBuffer, outFileLen, thumbnailBuffer, thumbnailLen, false, false); break;
		case str2int("gltf-drc"):
			exportResult = GLTFWrite(model, outFileBuffer, outFileLen, thumbnailBuffer, thumbnailLen, true, false); break;
		case str2int("glb"):
			exportResult = GLTFWrite(model, outFileBuffer, outFileLen, thumbnailBuffer, thumbnailLen, false, true); break;
		case str2int("glb-drc"):
			exportResult = GLTFWrite(model, outFileBuffer, outFileLen, thumbnailBuffer, thumbnailLen, true, true); break;
		default: exportResult = cnv_UnknownExtension;
		}

 		if (exportResult != cnv_Success) {
			switch (exportResult)
			{
			case cnv_NoBody:  errorMessage = "Bodies hasn't been found"; break;
			case cnv_NoObjects: errorMessage = "Objects hasn't been found"; break;
			case cnv_FileOpenError: errorMessage = "Cannot open file"; break;
			case cnv_ImpossibleReadAssembly: errorMessage = "Cannot read assembly"; break;
			case cnv_LicenseNotFound:  errorMessage = "Core license expired"; break;
			case cnv_UnknownExtension: errorMessage = "Unknown extention"; break;
			case cnv_NotEnoughMemory:  errorMessage = "Not enough memory"; break;
			case cnv_Error: errorMessage = "Unknown import error"; break;
			default: errorMessage = "Unknown error"; break;
			}

			return false;
		}

		return true;
	}
}