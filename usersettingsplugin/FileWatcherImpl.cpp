
#include "stdafx.h"
#include "FileWatcherImpl.h"

CFileWatcherImpl::CFileWatcherImpl()
{
}

CFileWatcherImpl::~CFileWatcherImpl()
{
}

long CFileWatcherImpl::OnFileChanged()
{
	std::cout << "onfilechange before()" << std::endl;
	*m_changeFlag = 1;
	std::cout << "onfilechange after()" << std::endl;
	return 0;
}
