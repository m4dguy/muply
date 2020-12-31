
#include "muply.h"

void readPly(const char* path) {
	printf("reading file: %s\n", path);
	// open file for reading
	PlyFile pfile = openPly(path);
	
	//print general file information
	printf("format: %s\n\n", (pfile.encoding == PlyEncoding::ASCII) ? "ascii" : "binary");
	PlyElement elem;
	size_t eCount = pfile.elementCount;
	PlyProperty* props;
	size_t pCount;
	for (size_t i = 0; i < eCount; ++i) {
		elem = pfile.elements[i];
		pCount = elem.propertyCount;
		props = elem.properties;
		printf("element name: %s (%zi)\n", elem.name, pCount);
		for (size_t p = 0; p < pCount; ++p) {
			const PlyProperty prop = props[p];
			printf("property name: %s (%zi)\n", prop.name, p);
		}
		printf("\n");
	}

	// request vertex data with full set of properties
	requestElement(&pfile, "vertex");
	// alternatively: request vertex data with specific set of properties
	//requestElement(&pfile, "vertex", 6, "x", "y", "z");
	requestElement(&pfile, "face");

	// show first 10 vertices in command line
	elem = pfile.elements[0];
	props = elem.properties;
	printf("show 10 of %zi items of element: %s\n", elem.itemCount, elem.name);
	const float* x = (float*)props[0].data;
	const float* y = (float*)props[1].data;
	const float* z = (float*)props[2].data;
	const float* intensity = (float*)props[4].data;
	for (size_t i = 0; i < 10; ++i) {
		printf("%f %f %f %f\n", x[i], y[i], z[i], intensity[i]);
	}
	printf("\n");

	// show first 10 faces in command line
	elem = pfile.elements[1];
	props = elem.properties;
	const uint8_t* li = (uint8_t*)props[0].listData;
	const int* fi = (int*)props[0].data;
	size_t idx = 0;
	printf("show 10 of %zi items of element: %s\n", elem.itemCount, elem.name);
	for (size_t i = 0; i < 10; ++i) {
		printf("%i ", li[i]);
		for (size_t j = 0; j < li[i]; ++j) {
			printf("%i ", fi[idx++]);
		}
		printf("\n");
	}

	// cleanup
	closePly(&pfile);
}

int main(){
	// test case
	const char* path = "./samples/bunny.ply";
	readPly(path);
	return 0;
}
