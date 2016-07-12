
#ifndef _FILEWATCHER_H_
#define _FILEWATCHER_H_

#ifndef __wtypes_h__
#include "WTypes.h"
#endif

class CFileWatcher
{
public:
    CFileWatcher();
    ~CFileWatcher();

public:
    long WatchFilePath(LPCTSTR szFilePath,long *changeFlag);
	long *m_changeFlag;

public:
    virtual long OnFileChanged(void);

private:
    HANDLE m_hThreadFile;
    TCHAR m_szPath[MAX_PATH];
};

#endif
