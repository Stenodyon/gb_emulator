// GBEmu.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CPU.h"
#include "registers.h"

#include <fstream>

const std::string filename = "pokemon.gb";

std::ifstream::pos_type filesize(const std::string& filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

int main()
{
#ifdef _DEBUG
	std::cout << "Debug build" << std::endl;
#endif
	uint64_t fileSize = filesize(filename);
	std::cout << "ROM is " << fileSize << "B" << std::endl;
	uint8_t * data = (uint8_t*)malloc(fileSize);
	if (data == NULL)
	{
		std::cerr << "Could not allocate memory for the ROM" << std::endl;
		return -1;
	}
	std::cout << "Allocated memory" << std::endl;
	std::ifstream romfile;
	romfile.open(filename, std::ios::in | std::ios::binary);
	romfile.read((char*)data, fileSize);
	romfile.close();
	std::cout << "Read file" << std::endl;
	CPU cpu = CPU(data);
#ifdef _DEBUG
	try {
#endif
		cpu.run();
#ifdef _DEBUG
	}
	catch (OpcodeNotImplemented e)
	{
		std::cerr << "Exception caught:" << std::endl;
		std::cerr << "Unimplemented OpCode: " << e.what() << std::endl;
	}
	catch (InvalidRegisterException e)
	{
		std::cerr << "Exception caught:" << std::endl;
		std::cerr << "Invalid register: " << e.what() << std::endl;
	}
#endif
	free(data);
    return 0;
}

