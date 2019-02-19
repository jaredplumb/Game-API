#include "Hermes.h"

bool Hermes::Build (const PString& path) {
	
	PXML xml;
	if(xml.NewFromFile(path) == false) {
		PSystem::Print("Failed to read xml hermes file!\n");
		return false;
	}
	
	PString packagePath = PString(path).TrimExtension() + ".package";
	PSystem::Print("Building %s...\n", (const char*)packagePath);
	
	PPackage package;
	if(package.OpenForWrite(packagePath) == false) {
		PSystem::Print("Failed to open package file!\n");
		return false;
	}
	
	for(std::multimap<PString, PXML*>::const_iterator i = xml.elements.begin(); i != xml.elements.end(); i++) {
		
		if(i->second == NULL)
			continue;
		
		// Find the initial files
		PString src = *i->second->GetAttribute("src");
		if(src.IsEmpty()) {
			src = i->second->GetContent();
			src.TrimSpaces();
		}
		if(src.IsEmpty()) {
			PSystem::Print("Element %s has no source!\n", (const char*)i->second->tag);
			continue;
		}
		
		// If there is a wildcard, adjust for the wildcard
		PString left;
		PString right;
		if(PString::strstr(src, "*") != NULL) {
			right = PString::strstr(src, "*") + 1;
			left = src;
			left[left.GetLength() - right.GetLength() - 1] = '\0';
			src.TrimToDirectory();
		}
		
		// Cycle through the directory building resources
		PDirectory dir(src);
		for(uint_t d = 0; d < dir.GetSize(); d++) {
			PString file = dir.GetFile(d);
			if((left.IsEmpty() && right.IsEmpty()) || (file.GetLength() >= left.GetLength() + right.GetLength() && PString::strncmp(file, left, left.GetLength()) == 0 && PString::strncmp(file + file.GetLength() - right.GetLength(), right, right.GetLength()) == 0)) {
				PString name = file;
				name[name.GetLength() - right.GetLength()] = '\0';
				PSystem::Print("Building %s %s as ", (const char*)i->second->tag, (const char*)name);
				name = name + left.GetLength();
				PSystem::Print("\"%s\"...\n", (const char*)name);
				
				// All the heavy lifting goes here
				if(PString::stricmp(i->second->tag, "images") == 0) {
					PImage::Resource image;
					if(image.NewFromFile(file) == false || image.WriteToPackage(package, name) == false)
						PSystem::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				//} else if(PString::stricmp(i->second->tag, "sounds") == 0) {
				//	PSoundResource sound;
				//	if(sound.NewFromFile(file))
				//		sound.Write(package, name);
				} else if(PString::stricmp(i->second->tag, "fonts") == 0) {
					PFontResource font;
					if(font.NewFromFile(file) == false || font.WriteToPackage(package, name) == false)
						PSystem::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				} else {
					PSystem::Print("Unknown file type \"%s\" for file \"%s\"!\n", (const char*)i->second->tag, (const char*)file);
				}
			}
		}
	}
	
	return true;
}
