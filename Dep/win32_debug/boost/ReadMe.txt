In order to compile this application you'll need to get the boost libraries.

1. To do so get the latest version from www.boost.org
2. Extract the files from the archieve to a folder.
3. open Visual studio command prompt and go to the folder you extracted the contents of the archieve in.
4. type /bootstrap
5. type /bjam toolset=msvc-10.0 variant=debug,release threading=multi link=static address-model=(Fill your need here 64 or 32)
6. Copy the contents of the folder to the location this file is in.
7. You've succesfully installed boost