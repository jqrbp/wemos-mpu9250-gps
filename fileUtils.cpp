#include <FS.h>
#include <LittleFS.h>

// Filesystem variables
FS* fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();

static bool fsOK = false;

bool isFileExist(const char * _path) {
    return fileSystem->exists(_path);
}

bool isfsOK() {
    return fsOK;
}

void fs_setup() {
  ////////////////////////////////
  // FILESYSTEM INIT

  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  Serial.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));
}

void appendFile(const char * path, const char * message) {
  // Serial.printf("Appending to file: %s\n", path);

  File file = fileSystem->open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (!file.print(message)) {
  //   Serial.println("Message appended");
  // } else {
    Serial.println("Append failed");
  }
  file.close();
}

void deleteRecursive(const char *path) {
  String pathStr = "";
  File file = fileSystem->open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir) {
    fileSystem->remove(path);
    return;
  }

  // Otherwise delete its contents first
  Dir dir = fileSystem->openDir(path);

  while (dir.next()) {
    pathStr = String(path) + "/" + dir.fileName();
    deleteRecursive(pathStr.c_str());
  }

  // Then delete the folder itself
  fileSystem->rmdir(path);
}