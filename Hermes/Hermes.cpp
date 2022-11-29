#include "Hermes.h"
#include "MyXML.h"
#include "GPackage.h"
#include "GSound.h"
#include "GImage.h"
#include "GFont.h"

bool Hermes::Build (const GString& path) {
	
	MyXML xml;
	if(xml.NewFromFile(path) == false) {
		GSystem::Print("Failed to read xml hermes file!\n");
		return false;
	}
	
	GString packagePath = GString(path).TrimExtension() + ".package";
	GSystem::Print("Building %s...\n", (const char*)packagePath);
	
	GPackage package;
	if(package.OpenForWrite(packagePath) == false) {
		GSystem::Print("Failed to open package file!\n");
		return false;
	}
	
	for(std::multimap<GString, MyXML*>::const_iterator i = xml.elements.begin(); i != xml.elements.end(); i++) {
		
		if(i->second == NULL)
			continue;
		
		// Find the initial files
		GString src = *i->second->GetAttribute("src");
		if(src.IsEmpty()) {
			src = i->second->GetContent();
			src.TrimSpaces();
		}
		if(src.IsEmpty()) {
			GSystem::Print("Element %s has no source!\n", (const char*)i->second->tag);
			continue;
		}
		
		// If there is a wildcard, adjust for the wildcard
		GString left;
		GString right;
		if(GString::strstr(src, "*") != NULL) {
			right = GString::strstr(src, "*") + 1;
			left = src;
			left[left.GetLength() - right.GetLength() - 1] = '\0';
			src.TrimToDirectory();
		}
		
		// Cycle through the directory building resources
		std::vector<GString> dir = GFile::GetFileNamesInDirectory(src);
		for(int d = 0; d < dir.size(); d++) {
			GString file = dir[d];
			if((left.IsEmpty() && right.IsEmpty()) || (file.GetLength() >= left.GetLength() + right.GetLength() && GString::strncmp(file, left, left.GetLength()) == 0 && GString::strncmp(file + file.GetLength() - right.GetLength(), right, right.GetLength()) == 0)) {
				GSystem::Print("Building \"%s\" as ", (const char*)file);
				GString name = file + left.GetLength();				// Removes the path
				name[name.GetLength() - right.GetLength()] = '\0';	// Removes the extension
				
				// All the heavy lifting goes here
				
				// Images
				if(GString::stricmp(i->second->tag, "images") == 0) {
					GSystem::Print("image \"%s\"...\n", (const char*)name);
					GImage::Resource image;
					if(image.NewFromFile(file) == false || image.WriteToPackage(package, name) == false)
						GSystem::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				
				// Sounds
				} else if(GString::stricmp(i->second->tag, "sounds") == 0) {
					GSystem::Print("sound \"%s\"...\n", (const char*)name);
					GSound::Resource sound;
					if(sound.NewFromFile(file) == false || sound.WriteToPackage(package, name) == false)
						GSystem::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				
				// Fonts
				} else if(GString::stricmp(i->second->tag, "fonts") == 0) {
					GSystem::Print("font \"%s\"...\n", (const char*)name);
					GFont::Resource font;
					if(font.NewFromFile(file) == false || font.WriteToPackage(package, name) == false)
						GSystem::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				
				// Data
				} else if(GString::stricmp(i->second->tag, "data") == 0) {
					GSystem::Print("data \"%s\"...\n", (const char*)(name + right));
					GFile data;
					uint8_t* buffer = NULL;
					int64_t size = 0;
					if(data.OpenForRead(file) == false || (size = data.GetSize()) == 0 || (buffer = new uint8_t[size]) == NULL ||
					   data.Read(buffer, size) == false || package.Write(name + right, buffer, size) == false)
						GSystem::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
					if(buffer != NULL)
						delete [] buffer;
				
				// Unknown
				} else {
					GSystem::Print("unknown file type \"%s\" (file not built)!\n", (const char*)i->second->tag);
				}
			}
		}
	}
	
	return true;
}
