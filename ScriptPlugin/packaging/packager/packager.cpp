// Generate an info.xml template from the version.h file.

#include <fstream>
#include <iostream>
#include <string>

#include <pluginsdk/PluginDefs.h>
#include <src/version.h>

using namespace std;

string escape(string str) {
	size_t i = 0;
	while((i = str.find_first_of("<>&", i)) != string::npos) {
		switch(str[i]) {
		case '<': str.replace(i, 1, "&lt;"); i += 4; break;
		case '>': str.replace(i, 1, "&gt;"); i += 4; break;
		case '&': str.replace(i, 1, "&amp;"); i += 5; break;
		default: ++i; break;
		}
	}
	return str;
}

enum { Path = 1, LastCompulsory = Path };

int main(int argc, char* argv[]) {
	if(argc <= LastCompulsory) {
		cout << "packager: no path given by the build script" << endl;
		return 1;
	}

	ofstream f(argv[Path]);
	if(!f.is_open()) {
		cout << "packager: cannot open " << argv[Path] << endl;
		return 2;
	}

	f << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n\n"
		"<!--\n"
		"This file is an example info.xml to be included in the dcext package. For more\n"
		"information, read \"Plugin format (dcext).txt\" in the \"doc\" directory.\n\n"
		"This file has been generated using the information filled in the src/version.h file.\n\n"
		"Edit the <Plugin> tags to include your plugin files.\n"
		"If you support Windows, include pe-x64 and pe-x86 platforms.\n"
		"If you support Linux, include elf-x64 and elf-x86 platforms.\n\n"
		"The <Files> tag is empty; should you want to distribute additional files, include them\n"
		"in there within <File> tags (again, more information in the above doc file).\n"
		"It is recommended you provide a .pdb file to ease debugging.\n\n"
		"When you are done editing this file, rename it to \"info.xml\", move the relevant files\n"
		"to this directory and zip them; rename that .zip to .dcext and you are done!\n"
		"-->\n\n"
		"<dcext>\n"
		"\t<UUID>" << escape(PLUGIN_GUID) << "</UUID>\n"
		"\t<Name>" << escape(PLUGIN_NAME) << "</Name>\n"
		"\t<Version>" << PLUGIN_VERSION << "</Version>\n"
		"\t<ApiVersion>" << DCAPI_CORE_VER << "</ApiVersion>\n"
		"\t<Author>" << escape(PLUGIN_AUTHOR) << "</Author>\n"
		"\t<Description>" << escape(PLUGIN_DESC) << "</Description>\n"
		"\t<Website>" << escape(PLUGIN_WEB) << "</Website>\n"
		"\t<Plugin Platform=\"elf-x64\">MyPlugin-x64.so</Plugin>\n"
		"\t<Plugin Platform=\"elf-x86\">MyPlugin-x86.so</Plugin>\n"
		"\t<Plugin Platform=\"pe-x64\">MyPlugin-x64.dll</Plugin>\n"
		"\t<Plugin Platform=\"pe-x86\">MyPlugin-x86.dll</Plugin>\n"
		"\t<Files>\n"
		"\t\t<File Platform=\"pe-x64\">MyPlugin-x64.pdb</File>\n"
		"\t\t<File Platform=\"pe-x86\">MyPlugin-x86.pdb</File>\n"
		"\t</Files>\n"
		"</dcext>\n";

	return 0;
}
