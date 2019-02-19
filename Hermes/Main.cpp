#include "Hermes.h"

int main (int argc, char* argv[]) {
	
	if(argc > 0 && PString::stricmp(argv[1], "-h") == 0) {
		PSystem::Print("\nUsage:\n");
		PSystem::Print("Hermes input_file ...\n");
		PSystem::Print("\tAny number of files may be converted into package files.\n\n");
		return 0;
	}
	
	PSystem::Print("\nHermes v%s\n", (const char*)Hermes::GetVersionString());
	
	for(int i = 1; i < argc; i++) {
		PSystem::Print("\nReading %s...\n", argv[i]);
		Hermes::Build(argv[i]);
		PSystem::Print("Done\n\n");
	}
	
	return 0;
}

