#ifndef __file_Utils_h__
#define __file_Utils_h__

bool isFileExist(const char * _path);
bool isfsOK();
void fs_setup();
void appendFile(const char * path, const char * message);
void writeFile(const char * path, const char * message);
void deleteRecursive(const char *path);

#endif // def(__file_Utils_h__)