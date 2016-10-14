/*
 * Copyright 2016 OSVR and contributors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "FileWatcher.h"
#include "stdafx.h" // Comment this off if pre-compiled header is not required.

#ifndef _INC_SHLWAPI
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

#ifndef _INC_TCHAR
#include <tchar.h>
#endif

HANDLE g_hFilePath = NULL;
TCHAR g_szFileName[MAX_PATH] = _T("");
TCHAR g_szFilePath[MAX_PATH] = _T("");
WIN32_FIND_DATA g_FileData = {};

bool ValidateFileChange() {
  bool Result = false;
  WIN32_FIND_DATA FileData = {};

  HANDLE hFile = FindFirstFile(g_szFilePath, &FileData);
  if (hFile != INVALID_HANDLE_VALUE) {
    if (memcmp(&g_FileData, &FileData, sizeof(WIN32_FIND_DATA)) != 0) {
      memcpy_s(&g_FileData, sizeof(WIN32_FIND_DATA), &FileData,
               sizeof(WIN32_FIND_DATA));

      Result = true;
    }

    FindClose(hFile);
  }

  return Result;
}

DWORD WINAPI Thread_MonitorFile(LPVOID lpParameter) {
  CFileWatcher *pThis = (CFileWatcher *)lpParameter;

  while (WaitForSingleObject(g_hFilePath, INFINITE) == WAIT_OBJECT_0) {
    if (ValidateFileChange() == true) {
      pThis->OnFileChanged();
    }

    if (FindNextChangeNotification(g_hFilePath) == FALSE) {
      // Some error, terminate the thread
      ExitThread(GetLastError());
    }
  }

  return 0;
}

CFileWatcher::CFileWatcher() : m_hThreadFile(NULL) {}

CFileWatcher::~CFileWatcher() {
  if ((m_hThreadFile != INVALID_HANDLE_VALUE) && (m_hThreadFile != NULL)) {
    TerminateThread(m_hThreadFile, 0);

    CloseHandle(m_hThreadFile);
    m_hThreadFile = NULL;
  }

  if ((g_hFilePath != INVALID_HANDLE_VALUE) && (g_hFilePath != NULL)) {
    FindCloseChangeNotification(g_hFilePath);

    g_hFilePath = NULL;
  }
}

long CFileWatcher::WatchFilePath(LPCTSTR szFilePath, long *changeFlag) {
  long Result = 0;

  m_changeFlag = changeFlag;

  if (PathFileExists(szFilePath) == TRUE) {
    _tcscpy_s(g_szFilePath, _tcslen(szFilePath) + 1, szFilePath);

    _tcscpy_s(g_szFileName, _tcslen(szFilePath) + 1, szFilePath);
    PathStripPath(g_szFileName);

    HANDLE hFile = FindFirstFile(szFilePath, &g_FileData);
    if (hFile != INVALID_HANDLE_VALUE) {
      FindClose(hFile);

      _tcscpy_s(m_szPath, _tcslen(szFilePath) + 1, szFilePath);
      PathRemoveFileSpec(m_szPath);

      /* g_hFilePath = FindFirstChangeNotification(m_szPath,
              FALSE,
              FILE_NOTIFY_CHANGE_FILE_NAME |
              FILE_NOTIFY_CHANGE_DIR_NAME |
              FILE_NOTIFY_CHANGE_ATTRIBUTES |
              FILE_NOTIFY_CHANGE_SIZE |
              FILE_NOTIFY_CHANGE_LAST_WRITE |
              FILE_NOTIFY_CHANGE_SECURITY);
      */

      g_hFilePath = FindFirstChangeNotification(
          m_szPath, FALSE,
          FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
              FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
              FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY);
      if (g_hFilePath == INVALID_HANDLE_VALUE) {
        Result = GetLastError();
      } else {
        m_hThreadFile =
            CreateThread(NULL, 0, Thread_MonitorFile, this, 0, NULL);
        if (m_hThreadFile == NULL) {
          Result = GetLastError();
        }
      }
    } else {
      Result = GetLastError();
    }
  } else {
    Result = ERROR_NOT_FOUND;
  }

  return Result;
}

// Overrides
long CFileWatcher::OnFileChanged(void) {
  // Do something if a file attributes changed.
  std::cout << "onfilefhange()" << std::endl;
  return 0;
}
