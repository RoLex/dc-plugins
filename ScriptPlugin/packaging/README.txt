This directory helps package a plugin into a .dcext file. The "Plugin format (dcext).txt" file in
the "doc" directory contains more information about the process.

The "packager" directory contains a helper script that generates an info.xml template to be used as
a base for the final info.xml which will be included in the .dcext archive.

Building the project should create an info_generated.xml file in this directory. Here follows the
comment block it will contain.

---

This file is an example info.xml to be included in the dcext package. For more
information, read "Plugin format (dcext).txt" in the "doc" directory.

This file has been generated using the information filled in the src/version.h file.

Edit the <Plugin> tags to include your plugin files.
If you support Windows, include pe-x64 and pe-x86 platforms.
If you support Linux, include elf-x64 and elf-x86 platforms.

The <Files> tag is empty; should you want to distribute additional files, include them
in there within <File> tags (again, more information in the above doc file).
It is recommended you provide a .pdb file to ease debugging.

When you are done editing this file, rename it to "info.xml", move the relevant files
to this directory and zip them; rename that .zip to .dcext and you are done!
