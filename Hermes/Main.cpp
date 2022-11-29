#include "Hermes.h"
#include "GSystem.h"

int main (int argc, char* argv[]) {
	
	if (argc > 0 && GString::stricmp(argv[1], "-h") == 0) {
		GSystem::Print("\nUsage:\n");
		GSystem::Print("Hermes input_file ...\n");
		GSystem::Print("\tAny number of files may be converted into package files.\n\n");
		return 0;
	}
	
	GSystem::Print("\nHermes v%s\n", (const char*)Hermes::GetVersionString());
	
	for (int i = 1; i < argc; i++) {
		GSystem::Print("\nReading %s...\n", argv[i]);
		Hermes::Build(argv[i]);
		GSystem::Print("Done\n\n");
	}
	
	return 0;
}
