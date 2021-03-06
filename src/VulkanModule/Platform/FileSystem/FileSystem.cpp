//
// FileSystem.cpp
//	Vulkan Platform Abstraction
//
// See matched header file for definitive main comment.
//
// Should be thread-safe if each thread instantiates its own.
//
// 5/19/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "FileSystem.h"
#include <fstream>


//string DATA_SUBDIRECTORY = "data/";
//const string _SHADER_SUBDIRECTORY  = DATA_SUBDIRECTORY+"compiledShaders/";
const StrPtr SHADER_SUBDIRECTORY=  "..//data/compiledShaders/";
//const string _TEXTURE_SUBDIRECTORY  = DATA_SUBDIRECTORY+"data/"textures/";
const StrPtr TEXTURE_SUBDIRECTORY ="..//data/textures/";


// Directory hierarchy project-specific conventions

string FileSystem::ShaderFileFullPath(StrPtr fileName)
{
	return ExeAccompaniedFullPath(fileName, SHADER_SUBDIRECTORY);
}

string FileSystem::TextureFileFullPath(StrPtr fileName)
{
	return ExeAccompaniedFullPath(fileName, TEXTURE_SUBDIRECTORY);
}


vector<char> FileSystem::ReadShaderFile(const string& shaderFilename)
{
	return readFile(shaderFilename, SHADER_SUBDIRECTORY);
}

vector<char> FileSystem::ReadTextureFile(const string& imageFilename)
{
	return readFile(imageFilename, TEXTURE_SUBDIRECTORY);
}


vector<char> FileSystem::readFile(const string& fileName, const char* subdirectoryName)
{
	const string fullPath = ExeAccompaniedFullPath(fileName, subdirectoryName);
	Log(NOTE, "reading file at path: " + fullPath);
	return readFile(fullPath);
}


// Standard Library file operations, hence generic.

vector<char> FileSystem::readFile(const string& pathName)
{
	std::ifstream file(pathName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		Fatal("Failed to open file: \"" + pathName + "\"!");

	size_t fileSize = (size_t) file.tellg();
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
