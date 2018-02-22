// GBEmu.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CPU.h"
#include "registers.h"

#include <fstream>

const std::string filename = "pokemon.gb";

template <typename T>
struct hex
{
	T value;

	hex(T value_arg) : value(value_arg) {}
};

template <typename T>
std::ostream & operator<<(std::ostream & out, hex<T> val)
{
	out << "0x" << std::uppercase << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +(val.value) << std::nouppercase;
	return out;
}

std::ifstream::pos_type filesize(const std::string& filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

void checkCGB(uint8_t * data)
{
	uint8_t cgb_flag = *(data + 0x0143);
	if ((cgb_flag & 0xC0) == 0x80)
		std::cout << "CGB mode optional" << std::endl;
	else if ((cgb_flag & 0xC0) == 0xC0)
		std::cout << "CGB mode required" << std::endl;
	else
		std::cout << "Non CGB mode" << std::endl;
}

void checkSGB(uint8_t * data)
{
	uint8_t sgb_flag = *(data + 0x0146);
	switch (sgb_flag)
	{
	case 0x00:
		std::cout << "No SGB" << std::endl;
		break;
	case 0x03:
		std::cout << "SGB supported" << std::endl;
		break;
	default:
		std::cout << "SGB disabled" << std::endl;
		break;
	}
}

void checkMBC(uint8_t * data)
{
	uint8_t mbc_type = *(data + 0x0147);
	switch (mbc_type)
	{
	case 0x00:
		std::cout << "ROM only" << std::endl;
		break;
	case 0x03:
		std::cout << "BATTERY ";
	case 0x02:
		std::cout << "RAM ";
	case 0x01:
		std::cout << "MBC1" << std::endl;
		break;
	case 0x06:
		std::cout << "BATTERY ";
	case 0x05:
		std::cout << "MBC2" << std::endl;
	default:
		std::cout << "Unknown MBC" << std::endl;
	}
}

void romInfo(uint8_t * data)
{
	char name[17]; name[16] = 0;
	uint8_t * name_ptr = data + 0x0134;
	for (uint8_t index = 0; index < 16; index++)
	{
		name[index] = *(name_ptr + index);
		if (name[index] == 0)
			break;
	}
	std::cout << "Game name: [" << name << "]" << std::endl;
	checkCGB(data);
	checkSGB(data);
	checkMBC(data);
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
	romInfo(data);
	//return 0; // ROM info
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
#endif
	free(data);
    return 0;
}

