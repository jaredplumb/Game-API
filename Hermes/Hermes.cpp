#include "Hermes.h"

bool Hermes::Build (const GString& path) {
	
	GXML xml;
	if(xml.NewFromFile(path) == false) {
		GConsole::Print("Failed to read xml hermes file!\n");
		return false;
	}
	
	GString packagePath = GString(path).TrimExtension() + ".package";
	GConsole::Print("Building %s...\n", (const char*)packagePath);
	
	GPackage package;
	if(package.OpenForWrite(packagePath) == false) {
		GConsole::Print("Failed to open package file!\n");
		return false;
	}
	
	for(std::multimap<GString, GXML*>::const_iterator i = xml.elements.begin(); i != xml.elements.end(); i++) {
		
		if(i->second == NULL)
			continue;
		
		// Find the initial files
		GString src = *i->second->GetAttribute("src");
		if(src.IsEmpty()) {
			src = i->second->GetContent();
			src.TrimSpaces();
		}
		if(src.IsEmpty()) {
			GConsole::Print("Element %s has no source!\n", (const char*)i->second->tag);
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
		GDirectory dir(src);
		for(uint_t d = 0; d < dir.GetSize(); d++) {
			GString file = dir.GetFile(d);
			if((left.IsEmpty() && right.IsEmpty()) || (file.GetLength() >= left.GetLength() + right.GetLength() && GString::strncmp(file, left, left.GetLength()) == 0 && GString::strncmp(file + file.GetLength() - right.GetLength(), right, right.GetLength()) == 0)) {
				GString name = file;
				name[name.GetLength() - right.GetLength()] = '\0';
				GConsole::Print("Building %s %s as ", (const char*)i->second->tag, (const char*)name);
				name = name + left.GetLength();
				GConsole::Print("\"%s\"...\n", (const char*)name);
				
				// All the heavy lifting goes here
				
				// Images
				if(GString::stricmp(i->second->tag, "images") == 0) {
					GImage::Resource image;
					if(image.NewFromFile(file) == false || image.WriteToPackage(package, name) == false)
						GConsole::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				
				// Sounds
				} else if(GString::stricmp(i->second->tag, "sounds") == 0) {
					GSound::Resource sound;
					if(sound.NewFromFile(file) == false || sound.WriteToPackage(package, name) == false)
						GConsole::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				
				// Fonts
				} else if(GString::stricmp(i->second->tag, "fonts") == 0) {
					GFont::Resource font;
					if(font.NewFromFile(file) == false || font.WriteToPackage(package, name) == false)
						GConsole::Print("Failed to create %s resource \"%s\"!\n", (const char*)i->second->tag, (const char*)name);
				
				// Unknown
				} else {
					GConsole::Print("Unknown file type \"%s\" for file \"%s\"!\n", (const char*)i->second->tag, (const char*)file);
				}
			}
		}
	}
	
	return true;
}
