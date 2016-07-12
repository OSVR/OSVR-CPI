
#include "FileWatcher.h"

class CFileWatcherImpl: public CFileWatcher
{
public:
    CFileWatcherImpl();
    ~CFileWatcherImpl();

public:
    // Implements virtual method
    long OnFileChanged(void);

};