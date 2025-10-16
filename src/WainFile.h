#ifndef _WAIN_FILE_H_INC
#define _WAIN_FILE_H_INC

class FileClass
{
protected:
  bool m_gotError;
public:
  FileClass()
  {
    m_gotError = FALSE;
  }
  virtual ~FileClass() = 0;
  virtual bool Error()
  {
    return m_gotError;
  }
  virtual GetStrLineEndType Read(char *ch) = 0;
  virtual bool Write(const char *s) = 0;
};

class StdioFileClass : public FileClass
{
  FILE *m_file;
public:
  StdioFileClass(const char *file_name, unsigned int access_mode);
  virtual ~StdioFileClass();
  virtual GetStrLineEndType Read(char *ch);
  virtual bool Write(const char *s);
};

class InetFileClass : public FileClass
{
  CInternetFile *m_inetFile;
public:
  InetFileClass(const char *file_name, unsigned int access_mode);
  virtual ~InetFileClass();
  virtual GetStrLineEndType Read(char *ch);
  virtual bool Write(const char *s);
};

class DebugFileClass : public FileClass
{
  HANDLE m_fileHandle;
  char m_leftOverChar = 0;
public:
  DebugFileClass(const char *file_name, unsigned int access_mode);
  virtual ~DebugFileClass();
  virtual GetStrLineEndType Read(char *ch);
  virtual bool Write(const char *s);
};

#endif