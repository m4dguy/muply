#include "muply.h"

PlyEncoding str2PlyEncoding(const char* str) {
	if (!strcmp(str, "ascii")) {
		return PlyEncoding::ASCII;
	}
	if (!strcmp(str, "binary_little_endian")) {
		return PlyEncoding::BINARY_LITTLE_ENDIAN;
	}
	if (!strcmp(str, "binary_big_endian")) {
		return PlyEncoding::BINARY_BIG_ENDIAN;
	}
	return PlyEncoding::UNKNOWN;
}

PlyType str2PlyType(const char* str) {
	if (!strcmp(str, "char") || !strcmp(str, "int8")) {
		return PlyType::INT8;
	}
	if (!strcmp(str, "short") || !strcmp(str, "int16")) {
		return PlyType::INT16;
	}
	if (!strcmp(str, "int") || !strcmp(str, "int32")) {
		return PlyType::INT32;
	}
	if (!strcmp(str, "long") || !strcmp(str, "int64")) {
		return PlyType::INT64;
	}
	if (!strcmp(str, "uchar") || !strcmp(str, "uint8")) {
		return PlyType::UINT8;
	}
	if (!strcmp(str, "ushort") || !strcmp(str, "uint16")) {
		return PlyType::UINT16;
	}
	if (!strcmp(str, "ulong") || !strcmp(str, "uint64")) {
		return PlyType::UINT64;
	}
	if (!strcmp(str, "float") || !strcmp(str, "float32")) {
		return PlyType::FLOAT32;
	}
	if (!strcmp(str, "double") || !strcmp(str, "float64")) {
		return PlyType::FLOAT64;
	}
	return PlyType::UNKOWN;
}

bool isLittleEndian() {
	int num = 1;
	return (*(char*)&num == 1);
}

void byteSwap16(void* ptr, const size_t elementCount) {
	int16_t valIn, valOut;
	int16_t* ptr16 = (int16_t*)ptr;
	for (size_t i = 0; i < elementCount; ++i) {
		valIn = ptr16[i];
		((int8_t*)(&valOut))[0] = ((int8_t*)(&valIn))[1];
		((int8_t*)(&valOut))[1] = ((int8_t*)(&valIn))[0];
		ptr16[i] = valOut;
	}
}

void byteSwap32(void* ptr, const size_t elementCount) {
	int32_t valIn, valOut;
	int32_t* ptr32 = (int32_t*)ptr;
	for (size_t i = 0; i < elementCount; ++i) {
		valIn = ptr32[i];
		((int8_t*)(&valOut))[0] = ((int8_t*)(&valIn))[3];
		((int8_t*)(&valOut))[1] = ((int8_t*)(&valIn))[2];
		((int8_t*)(&valOut))[2] = ((int8_t*)(&valIn))[1];
		((int8_t*)(&valOut))[3] = ((int8_t*)(&valIn))[0];
		ptr32[i] = valOut;
	}
}

void byteSwap64(void* ptr, const size_t elementCount) {
	int64_t valIn, valOut;
	int64_t* ptr64 = (int64_t*)ptr;
	for (size_t i = 0; i < elementCount; ++i) {
		valIn = ptr64[i];
		((int8_t*)(&valOut))[0] = ((int8_t*)(&valIn))[7];
		((int8_t*)(&valOut))[1] = ((int8_t*)(&valIn))[6];
		((int8_t*)(&valOut))[2] = ((int8_t*)(&valIn))[5];
		((int8_t*)(&valOut))[3] = ((int8_t*)(&valIn))[4];
		((int8_t*)(&valOut))[4] = ((int8_t*)(&valIn))[3];
		((int8_t*)(&valOut))[5] = ((int8_t*)(&valIn))[2];
		((int8_t*)(&valOut))[6] = ((int8_t*)(&valIn))[1];
		((int8_t*)(&valOut))[7] = ((int8_t*)(&valIn))[0];
		ptr64[i] = valOut;
	}
}

PlyFile openPly(const char* path) {
	PlyFile pfile;
	pfile.file = fopen(path, "rb");
	// check file existence
	if (!pfile.file) {
		return pfile;
	}
	// allocate buffer for reading
	char* token;
	char buffer[MUPLY_BUFFER_SIZE];
	// check file validity
	fgets(buffer, MUPLY_BUFFER_SIZE, pfile.file);
	if (strcmp(buffer, "ply\n")) {
		fclose(pfile.file);
		pfile.file = NULL;
		return pfile;
	}
	// get format and encoding, count elements, allocate element memory
	pfile.elementCount = 0;
	while (!feof(pfile.file)) {
		fgets(buffer, MUPLY_BUFFER_SIZE, pfile.file);
		token = strtok(buffer, MUPLY_TOKEN_SEP);
		if (!strcmp(token, "format")) {
			// get format
			token = strtok(NULL, MUPLY_TOKEN_SEP);
			pfile.encoding = str2PlyEncoding(token);
			continue;
		}
		if (!strcmp(token, "element")) {
			// count number of elements
			pfile.elementCount += 1;
			continue;
		}
		if (!strcmp(token, "end_header")) {
			break;
		}
	}
	pfile.elements = (PlyElement*)malloc(pfile.elementCount * sizeof(PlyElement));
	// count per-element properties and allocate property memory
	PlyElement elem;
	int elementIdx = -1;
	pfile.dataStart = ftell(pfile.file);
	fseek(pfile.file, 0, SEEK_SET);
	while (!feof(pfile.file)) {
		fgets(buffer, MUPLY_BUFFER_SIZE, pfile.file);
		token = strtok(buffer, MUPLY_TOKEN_SEP);
		if (!strcmp(token, "element")) {
			// initialize element
			if (elementIdx >= 0) {
				elem.properties = (PlyProperty*)malloc(elem.propertyCount * sizeof(PlyProperty));
				pfile.elements[elementIdx] = elem;
			}
			++elementIdx;
			elem.propertyCount = 0;
			// get element name
			token = strtok(NULL, MUPLY_TOKEN_SEP);
			elem.nameLength = strlen(token);
			elem.name = (char*)malloc(elem.nameLength + 1);
			strcpy(elem.name, token);
			// get number elements
			token = strtok(NULL, MUPLY_TOKEN_SEP);
			elem.itemCount = atoi(token);
			continue;
		}
		if (!strcmp(token, "property")) {
			// count element properties
			elem.propertyCount += 1;
			continue;
		}
		if (!strcmp(token, "end_header")) {
			break;
		}
	}
	pfile.elements[elementIdx] = elem;
	// populate per-element properties
	elementIdx = -1;
	size_t propertyIdx = 0;
	fseek(pfile.file, 0, SEEK_SET);
	while (!feof(pfile.file)) {
		fgets(buffer, MUPLY_BUFFER_SIZE, pfile.file);
		token = strtok(buffer, MUPLY_TOKEN_SEP);
		if (!strcmp(token, "element")) {
			// switch to next element and initialize properties
			if (elementIdx >= 0) {
				pfile.elements[elementIdx] = elem;
			}
			++elementIdx;
			propertyIdx = 0;
			elem = pfile.elements[elementIdx];
			elem.properties = (PlyProperty*)malloc(elem.propertyCount * sizeof(PlyProperty));
			continue;
		}
		if (!strcmp(token, "property")) {
			// get property
			PlyProperty prop;
			token = strtok(NULL, MUPLY_TOKEN_SEP);
			if (!strcmp(token, "list")) {				
				// set as list if necessary
				token = strtok(NULL, MUPLY_TOKEN_SEP);
				prop.listType = str2PlyType(token);
				token = strtok(NULL, MUPLY_TOKEN_SEP);
			}
			else {
				prop.listType = PlyType::NONE;
			}
			// get property type
			prop.type = str2PlyType(token);
			token = strtok(NULL, MUPLY_TOKEN_SEP);
			prop.nameLength = strlen(token);
			prop.name = (char*)malloc(prop.nameLength + 1);
			strcpy(prop.name, token);			
			elem.properties[propertyIdx++] = prop;
			continue;
		}
		if (!strcmp(token, "end_header")) {
			break;
		}
	}
	pfile.elements[elementIdx] = elem;
	inspectData(&pfile);
	return pfile;
}

void inspectData(PlyFile* file) {
	// forward to suitable inspection function
	if (file->encoding == PlyEncoding::ASCII) {
		inspectDataAscii(file);
	}
	else {
		inspectDataBinary(file);
	}
}

void inspectDataAscii(PlyFile* file) {
	// skip to global data start
	fseek(file->file, file->dataStart, SEEK_SET);
	// buffer setup
	char buffer[MUPLY_BUFFER_SIZE];
	// setup
	size_t itemSize;
	long dataStart = file->dataStart;
	bool fixedLength;
	// get block size of each element
	int64_t listElements = 0;
	char* token;
	PlyElement elem;
	const size_t eCount = file->elementCount;
	PlyProperty prop;
	PlyProperty* props;
	size_t pCount, iCount;
	for (size_t e = 0; e < eCount; ++e) {
		// property setup
		fixedLength = true;
		elem = file->elements[e];
		elem.dataStart = dataStart;
		props = elem.properties;
		pCount = elem.propertyCount;
		iCount = elem.itemCount;
		// check for variable length properties
		for (size_t p = 0; p < pCount; ++p) {
			fixedLength &= (props[p].listType == PlyType::NONE);
		}
		// inspect fixed-size property blocks
		if (fixedLength) {
			// set sizes of each property block
			for (size_t p = 0; p < pCount; ++p) {
				prop = props[p];
				props[p].propertySize = (long)(elem.itemCount * PlyTypeSizes[prop.type]);
			}
			// skip through block of elements
			for (size_t i = 0; i < elem.itemCount; ++i) {
				fgets(buffer, MUPLY_BUFFER_SIZE, file->file);
			}
		}
		else {
			// forward skips of varying length in case of non-fixed length
			for (size_t i = 0; i < iCount; ++i) {
				fgets(buffer, MUPLY_BUFFER_SIZE, file->file);
				token = strtok(buffer, MUPLY_TOKEN_SEP);
				for (size_t p = 0; p < pCount; ++p) {
					prop = elem.properties[p];
					itemSize = PlyTypeSizes[prop.type];
					// treat list case
					if (prop.listType != PlyType::NONE) {
						listElements = (int64_t)atol(token);
						prop.propertySize += (long)(listElements * itemSize);
						// skip list entries
						for (int s = 0; s < listElements; ++s) {
							strtok(NULL, MUPLY_TOKEN_SEP);
						}
					}
					else {
						prop.propertySize += (long)itemSize;
					}
					// get next property element
					token = strtok(NULL, MUPLY_TOKEN_SEP);
					props[p] = prop;
				}
			}
		}
		dataStart = ftell(file->file);
		file->elements[e] = elem;
	}
}

void inspectDataBinary(PlyFile* file) {
	// skip to global data start
	fseek(file->file, file->dataStart, SEEK_SET);
	// setup
	size_t listTypeSize, blockSize, itemSize;
	long dataStart = file->dataStart;
	bool fixedLength;
	// get block size of each element
	int64_t listElements = 0;
	PlyElement elem;
	const size_t eCount = file->elementCount;
	PlyProperty prop;
	PlyProperty* props;
	size_t pCount, iCount;
	for (size_t e = 0; e < eCount; ++e) {
		// property setup
		fixedLength = true;
		elem = file->elements[e];
		elem.dataStart = dataStart;
		props = elem.properties;
		pCount = elem.propertyCount;
		iCount = elem.itemCount;
		// check for variable length properties
		for (size_t p = 0; p < pCount; ++p) {
			prop = elem.properties[p];
			if (prop.listType != PlyType::NONE) {
				fixedLength = false;
			}
		}
		// estimate property blocks
		if (fixedLength) {
			// calculate fixed-size block dimensions
			blockSize = 0;
			for (size_t p = 0; p < pCount; ++p) {
				prop = props[p];
				prop.propertySize = (long)(elem.itemCount * PlyTypeSizes[prop.type]);
				blockSize += prop.propertySize;
				props[p] = prop;
			}
			// fast seek ahead by fixed size for binary files
			fseek(file->file, (long)(blockSize), SEEK_CUR);
		}
		else {
			// forward skips of varying length in case of non-fixed length
			for (size_t i = 0; i < iCount; ++i) {
				// skim through each property individually
				for (size_t p = 0; p < pCount; ++p) {
					prop = elem.properties[p];
					itemSize = PlyTypeSizes[prop.type];
					if (prop.listType != PlyType::NONE) {
						// deal with list case
						listTypeSize = PlyTypeSizes[prop.listType];
						fread(&listElements, listTypeSize, 1, file->file);
						prop.propertySize += (long)(listElements * itemSize);
						fseek(file->file, (long)(listElements * itemSize), SEEK_CUR);
					}
					else {
						// deal with non-list case
						prop.propertySize += (long)itemSize;
						fseek(file->file, (long)(itemSize), SEEK_CUR);
					}
					props[p] = prop;
				}
			}
		}
		dataStart = ftell(file->file);
		file->elements[e] = elem;
	}
}

void closePly(PlyFile* file) {
	// close source file
	fclose(file->file);
	file->file = NULL;
	// free elements and properties
	const size_t eCount = file->elementCount;
	PlyElement* elems = file->elements;
	PlyElement elem;
	size_t pCount;
	PlyProperty* props;
	PlyProperty prop;
	for (size_t e = 0; e < eCount; ++e) {
		elem = elems[e];
		pCount = elem.propertyCount;
		props = elem.properties;
		for (size_t p = 0; p < pCount; ++p) {
			prop = props[p];
			if (prop.name) {
				free(prop.name);
			}
			if (prop.listData) {
				free(prop.listData);
			}
			if (prop.data) {
				free(prop.data);
			}
		}
		if (props) {
			free(props);
		}
		if (elem.name) {
			free(elem.name);
		}
	}
	free(elems);
	// reset attributes
	file->encoding = PlyEncoding::UNKNOWN;
	file->elements = NULL;
	file->elementCount = 0;
	file->dataStart = 0;
}

bool requestElement(PlyFile* file, const char* name, size_t n, ...) {
	// get element index by name
	int elemIdx = -1;
	PlyElement elem;
	const size_t elementCount = file->elementCount;
	for (size_t i = 0; i < elementCount; ++i) {
		elem = file->elements[i];
		if (!strcmp(elem.name, name)) {
			elemIdx = (int)i;
			break;
		}
	}
	if (elemIdx == -1) {
		// element name not found
		return false;
	}
	// get properties
	const size_t eCount = elem.itemCount;
	const size_t pCount = elem.propertyCount;
	PlyProperty* props = elem.properties;
	// allocate memory for requested properties
	bool requestAll = !n;
	PlyProperty prop;
	int requestIdx;
	char* requestName;
	va_list vl;
	va_start(vl, n);
	if (requestAll) {
		n = pCount;
	}
	// allocate memory for requested properties
	int nAllocated = 0;
	for (size_t i = 0; i < n; ++i) {
		requestIdx = (int)i;
		if (!requestAll) {
			// try find index of requested property
			requestIdx = -1;
			requestName = va_arg(vl, char*);
			for (size_t p = 0; p < pCount; ++p) {
				prop = props[p];
				if (!strcmp(prop.name, requestName)) {
					requestIdx = (int)p;
					break;
				}
			}
		}
		if (requestIdx != -1) {
			// allocate memory of requested property
			prop = props[requestIdx];
			if (!prop.data) {
				// allocate raw data space
				prop.data = malloc(prop.propertySize);
			}
			if (!prop.listData && (prop.listType != PlyType::NONE)) {
				// allocate raw list index space
				// raw data space will be allocated when reading the property
				prop.listData = malloc(elem.itemCount * PlyTypeSizes[prop.listType]);
			}
			props[requestIdx] = prop;
			++nAllocated;
		}
	}
	va_end(vl);
	// get endianness
	const bool endianSys = isLittleEndian();
	const bool endianData = (file->encoding == PlyEncoding::BINARY_LITTLE_ENDIAN);
	const bool needByteSwap = (endianSys != endianData);
	// forward to suitable read function
	if (nAllocated) {
		switch (file->encoding) {
		case PlyEncoding::ASCII:
			readPropertiesAscii(file, elemIdx);
			break;
		case PlyEncoding::BINARY_LITTLE_ENDIAN:
		case PlyEncoding::BINARY_BIG_ENDIAN:
			readPropertiesBinary(file, elemIdx);
			//byteswap data if necessary
			if (needByteSwap) {
				byteSwapProperties(file, elemIdx);
			}
			break;
		default:
			break;
		}
	}
	return true;
}

void readPropertiesAscii(PlyFile* file, const size_t elemIdx) {
	// setup properties and jump to element block
	PlyElement elem = file->elements[elemIdx];
	PlyProperty* props = elem.properties;
	const size_t iCount = elem.itemCount;
	const size_t pCount = elem.propertyCount;
	fseek(file->file, elem.dataStart, SEEK_SET);
	// allocate buffer for reading
	char buffer[MUPLY_BUFFER_SIZE];
	// setup position indices for the properties
	for (size_t p = 0; p < pCount; ++p) {
		props[p].propertySize = 0;
	}
	// setup properties
	char* token;
	size_t itemSize;
	int listElements, pIdx;
	int8_t* data;
	uint8_t* data8u; uint16_t* data16u; uint32_t* data32u; uint64_t* data64u;
	int8_t* data8i; int16_t* data16i; int32_t* data32i; int64_t* data64i;
	float* data32f; double* data64f;
	PlyProperty prop;
	// read data
	for (size_t i = 0; i < iCount; ++i) {
		// get line and split tokens
		fgets(buffer, MUPLY_BUFFER_SIZE, file->file);
		token = strtok(buffer, MUPLY_TOKEN_SEP);
		for (size_t p = 0; p < pCount; ++p) {
			prop = props[p];
			if (prop.data) {
				listElements = 1;
				pIdx = prop.propertySize;
				// get list element if necessary
				if (prop.listType != PlyType::NONE) {
					listElements = atoi(token);
					itemSize = PlyTypeSizes[prop.listType];
					data = (int8_t*)prop.listData;
					memcpy(data + i * itemSize, &listElements, itemSize);
					token = strtok(NULL, MUPLY_TOKEN_SEP);
				}
				// switch over data type to convert token appropriately
				switch (prop.type) {
				case PlyType::UINT8:
					data8u = (uint8_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data8u[pIdx++] = (uint8_t)atoi(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}					
					break;
				case PlyType::UINT16:
					data16u = (uint16_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data16u[pIdx++] = (uint16_t)atoi(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::UINT32:
					data32u = (uint32_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data32u[pIdx++] = (uint32_t)atoi(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::UINT64:
					data64u = (uint64_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data64u[pIdx++] = (uint64_t)atol(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::INT8:
					data8i = (int8_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data8i[pIdx++] = (int8_t)atoi(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::INT16:
					data16i = (int16_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data16i[pIdx++] = (int16_t)atoi(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::INT32:
					data32i = (int32_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data32i[pIdx++] = (int32_t)atoi(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::INT64:
					data64i = (int64_t*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data64i[pIdx++] = (int64_t)atol(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::FLOAT32:
					data32f = (float*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data32f[pIdx++] = (float)atof(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				case PlyType::FLOAT64:
					data64f = (double*)prop.data;
					for (int l = 0; l < listElements; ++l) {
						data64f[pIdx++] = (double)atof(token);
						token = strtok(NULL, MUPLY_TOKEN_SEP);
					}
					break;
				default:
					break;
				}
				props[p].propertySize += (long)listElements;
			}			
		}
	}
	// restore property sizes after abusing them
	for (size_t p = 0; p < pCount; ++p) {
		prop = props[p];
		if (prop.data) {
			props[p].propertySize *= (long)PlyTypeSizes[prop.type];
		}
	}
}

void readPropertiesBinary(PlyFile* file, const size_t elemIdx) {
	// setup properties and jump to element block
	PlyElement elem = file->elements[elemIdx];
	PlyProperty* props = elem.properties;
	const size_t iCount = elem.itemCount;
	const size_t pCount = elem.propertyCount;
	fseek(file->file, elem.dataStart, SEEK_SET);
	// setup position indices for the properties
	for (size_t p = 0; p < pCount; ++p) {
		props[p].propertySize = 0;
	}
	// prepare properties
	int64_t listElements = 0;
	size_t readSize, pIdx;
	int8_t* data;
	PlyProperty prop;
	// read data
	for (size_t i = 0; i < iCount; ++i) {
		for (size_t p = 0; p < pCount; ++p) {
			prop = props[p];
			if (prop.data) {				
				listElements = 1;
				pIdx = prop.propertySize;
				if (prop.listType != PlyType::NONE) {
					data = (int8_t*)prop.listData;
					readSize = PlyTypeSizes[prop.listType];
					fread(&listElements, readSize, 1, file->file);
					memcpy(data + i * readSize, &listElements, readSize);
				}
				// read requested property
				data = (int8_t*)prop.data;
				readSize = PlyTypeSizes[prop.type];
				fread(data + pIdx * readSize, readSize, listElements, file->file);
			}
			else {
				// skip unrequested property
				readSize = PlyTypeSizes[prop.type] * listElements;
				fseek(file->file, (long)readSize, SEEK_CUR);
			}
			props[p].propertySize += (long)listElements;
		}
	}
	// restore property sizes after abusing them
	for (size_t p = 0; p < pCount; ++p) {
		prop = props[p];
		if (prop.data) {
			props[p].propertySize *= (long)PlyTypeSizes[prop.type];
		}
	}	
}

void byteSwapProperties(PlyFile* file, const size_t elemIdx) {
	PlyElement elem = file->elements[elemIdx];
	PlyProperty prop;
	PlyProperty* props = elem.properties;
	const size_t pCount = elem.propertyCount;
	size_t iCount;
	for (size_t p = 0; p < pCount; ++p) {
		prop = props[p];
		iCount = prop.propertySize / PlyTypeSizes[prop.type];
		switch (prop.type) {
		case PlyType::INT16:
		case PlyType::UINT16:
			byteSwap16(prop.data, iCount);
			break;
		case PlyType::INT32:
		case PlyType::UINT32:
		case PlyType::FLOAT32:
			byteSwap32(prop.data, iCount);
			break;
		case PlyType::FLOAT64:
			byteSwap64(prop.data, iCount);
			break;
		case PlyType::INT8:
		case PlyType::UINT8:
		default:
			// nothing to do
			break;
		}
	}
}
