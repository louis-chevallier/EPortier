#pragma ONCE

#include "LittleFS.h"

inline void listAllFilesInDir(const String &dir_path)
{
	Dir dir = LittleFS.openDir(dir_path);
	while(dir.next()) {
		if (dir.isFile()) {
                  // print file names
                  //EKOT(dir_path + dir.fileName());
		}
		if (dir.isDirectory()) {
			// print directory names
                  EKOT(dir_path + dir.fileName() + "/");
                  // recursive file listing inside new directory
                  listAllFilesInDir(dir_path + dir.fileName() + "/");
		}
	}
}

inline String read_file(const String &fn) {
  String s;
  
  {
    File file = LittleFS.open(fn, "r");
    if (file != 0) {
      while (file.available()) {
        auto c = file.read();
        s += String((char) c);
      }
      //EKOX(s);
      file.close();
    }
  }
  return s;
}

