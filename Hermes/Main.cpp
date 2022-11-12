#include "Hermes.h"

int main (int argc, char* argv[]) {
	
	if (argc > 0 && GString::stricmp(argv[1], "-h") == 0) {
		GConsole::Print("\nUsage:\n");
		GConsole::Print("Hermes input_file ...\n");
		GConsole::Print("\tAny number of files may be converted into package files.\n\n");
		return 0;
	}
	
	GConsole::Print("\nHermes v%s\n", (const char*)Hermes::GetVersionString());
	
	for (int i = 1; i < argc; i++) {
		GConsole::Print("\nReading %s...\n", argv[i]);
		Hermes::Build(argv[i]);
		GConsole::Print("Done\n\n");
	}
	
	return 0;
}
