#ifndef MUPLY_H
#define MUPLY_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// size of buffer for reading ascii data
#define MUPLY_BUFFER_SIZE 256
// delimiter tokens
#define MUPLY_TOKEN_SEP " \r\n"

/*
* Data types.
*/
enum PlyType {
	UNKOWN, NONE,
	INT8, INT16, INT32, INT64,
	UINT8, UINT16, UINT32, UINT64,
	FLOAT32, FLOAT64
};
/*
* Size conversion table for data types.
*/
const size_t PlyTypeSizes[] = {
	0, 0,
	sizeof(int8_t), sizeof(int16_t), sizeof(int32_t), sizeof(int64_t),
	sizeof(uint8_t), sizeof(uint16_t), sizeof(uint32_t), sizeof(uint64_t),
	sizeof(float), sizeof(double)
};
/*
* String conversion table for data types.
*/
const char PlyTypeStrings[12][10] = {
	"unknown", "none",
	"int8", "int16", "int32", "int64",
	"uint8", "uint16", "uint32", "uint64",
	"float32", "float64"
};
/*
* Encoding types.
*/
enum PlyEncoding {
	UNKNOWN,
	ASCII,
	BINARY_LITTLE_ENDIAN,
	BINARY_BIG_ENDIAN
};
/*
* Property fields.
*/
struct PlyProperty {
	// type of the property
	PlyType type = PlyType::UNKOWN;
	// property name
	char* name = NULL;
	// length of property name
	size_t nameLength = 0;
	// type of list elements (if list attribute exists)
	PlyType listType = PlyType::NONE;
	// pointer to list data with number of elements per entry
	// only defined for list entries and if list attribute exists
	void* listData = NULL;
	// pointer to data (if read)
	void* data = NULL;
	// size of property memory block
	long propertySize = 0;
};
/*
* Element fields.
*/
struct PlyElement {
	// element name
	char* name = NULL;
	// length of element name
	size_t nameLength = 0;
	// number of items
	size_t itemCount = 0;
	// properties of element
	PlyProperty* properties = NULL;
	// number of properties
	size_t propertyCount = 0;
	// start of element block in file
	long dataStart = 0;
};
/*
* Container for basic file information.
*/
struct PlyFile {
	// file pointer
	FILE* file = NULL;
	// encoding type
	PlyEncoding encoding = PlyEncoding::UNKNOWN;
	// number of elements in file
	int elementCount = 0;
	// element data
	PlyElement* elements = NULL;
	// start of data section
	long dataStart = 0;
};
/*
* Convert c-string to PlyEncoding.
* @param str c-string for conversion.
* @return Suitable PlyEncoding.
*/
PlyEncoding str2PlyEncoding(const char* str);
/*
* Convert c-string to PlyType.
* @param str c-string for conversion.
* @return Suitable PlyType.
*/
PlyType str2PlyType(const char* str);
/*
* Check if system is little endian.
* @return True, if system is little endian.
*/
bool isLittleEndian();
/*
* Do byteswapping of 16bit data.
* @param ptr Pointer to original data.
* @param elementCount Number of elements to byteswap.
*/
void byteSwap16(void* ptr, const size_t elementCount);
/*
* Do byteswapping of 32bit data.
* @param ptr Pointer to original data.
* @param elementCount Number of elements to byteswap.
*/
void byteSwap32(void* ptr, const size_t elementCount);
/*
* Do byteswapping of 64bit data.
* @param ptr Pointer to original data.
* @param elementCount Number of elements to byteswap.
*/
void byteSwap64(void* ptr, const size_t elementCount);
/*
* Open file and get basic information from header section.
* The PlyFile object will be reused for data queries.
* @param path Path to file.
* @return PlyFile object with basic file information. File information will be empty if loading failed.
*/
PlyFile openPly(const char* path);
/*
* Scan the ply file data and create an index of available properties and data blocks.
* Forwards to inspectDataAscii or inspectDataBinary.
* @param file PlyFile for inspection.
*/
void inspectData(PlyFile* file);
/*
* Scan the ply file data and create an index of available properties and data blocks.
* The results are written directly to the PlyFile object.
* @param file PlyFile for inspection.
*/
void inspectDataAscii(PlyFile* file);
/*
* Scan the ply file data and create an index of available properties and data blocks.
* The results are written directly to the PlyFile object.
* @param file PlyFile for inspection.
*/
void inspectDataBinary(PlyFile* file);
/*
* Close file and release all loaded ply data.
* @param PlyFile object to be closed.
*/
void closePly(PlyFile* file);
/*
* Request specific element and properties from file to be read.
* Define the number of requested properties and their names for loading.
* The loaded data will be written to the buffers of each PlyProperty object.
* If the file has not been inspected beforehand, it will be inspectData will be called.
* @param file PlyFile object for reading.
* @param name Name of property to be loaded.
* @param n Optional parameter with number of requested properties. Omit or set to 0 to load entire vertex data.
* @param ... C-strings identifying the names of the requested properties.
* @return True, if target property was found and loaded.
*/
bool requestElement(PlyFile* file, const char* name, size_t n = 0, ...);
/*
* Internally used to read vertex data from ascii-based ply files.
* @param file PlyFile object prepared for reading.
*/
void readPropertiesAscii(PlyFile* file, const size_t elemIdx);
/*
* Internally used to read vertex data from binary-based ply files.
* Performs all necessary byteswaps after reading.
* @param file PlyFile object prepared for reading.
*/
void readPropertiesBinary(PlyFile* file, const size_t elemIdx);
/*
* Inplace-byteswap properties of element data.
* @file PlyFile for byteswapping.
* @elemIdx Index of element for byteswapping.
*/
void byteSwapProperties(PlyFile* file, const size_t elemIdx);
#endif