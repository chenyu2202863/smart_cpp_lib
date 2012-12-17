#include "ShellLink.h"
#include <shobjidl.h>
#include <ShlGuid.h>
#include "pidl.h"
#include <string>
#include <CommonControls.h>

HRESULT OS::CreateShortcut(LPCTSTR lpszPathLink, LPCTSTR lpszPathObj, LPCTSTR lpszDesc)
{
	IShellLink* psl = NULL;

	// Get a pointer to the IShellLink interface.
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		// Set the path to the shortcut target and add the description.
		psl->SetPath(lpszPathObj);
		if (lpszDesc)
		{
			psl->SetDescription(lpszDesc);
		}		

		IPersistFile* ppf = NULL;
		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(lpszPathLink, TRUE);
			ppf->Release();
		}
		psl->Release();
	}

	return hres;
}

BOOL OS::ShowContextMenu(HWND hwnd, LPCTSTR pszPath, POINT point)
{
	if (hwnd == NULL || pszPath == NULL)
	{
		return FALSE;
	}

	WCHAR wchPath[MAX_PATH] = {0};
#ifdef _UNICODE
	::GetFullPathName(pszPath, sizeof(wchPath)/sizeof(wchPath[0]), wchPath, NULL); 
#else
	TCHAR   tchPath[MAX_PATH];
	::GetFullPathName(pszPath, sizeof(tchPath)/sizeof(tchPath[0]), tchPath, NULL);
	::MultiByteToWideChar(CP_ACP, 0, tchPath, -1, wchPath, sizeof(wchPath)/sizeof(wchPath[0]));
#endif

	BOOL bResult = FALSE;
	LPSHELLFOLDER psfFolder = NULL;
	LPITEMIDLIST pidlItem = NULL;
	if (GetShellFolderAndRelativeItemidlist(wchPath, &psfFolder, &pidlItem))
	{ 
		LPCONTEXTMENU pContextMenu = NULL;
		if (SUCCEEDED(psfFolder->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, NULL, (LPVOID*)&pContextMenu)))
		{
			bResult = PopContextMenu(hwnd, point, pContextMenu);
			pContextMenu->Release();
		}	

		PIDL::ILFree(pidlItem);
		psfFolder->Release(); 
	}
	
	return bResult; 
}

BOOL OS::ShowContextMenu(HWND hwnd, const std::vector<std::wstring> &vctFilePaths, POINT point)
{
	if (hwnd == NULL || vctFilePaths.empty())
	{
		return FALSE;
	}

	LPITEMIDLIST *idList = new LPITEMIDLIST[vctFilePaths.size()];
	int nIDCount = 0;
	std::vector<std::wstring>::const_iterator ite = vctFilePaths.begin();

	LPSHELLFOLDER psfFolder = NULL;
	for (; ite != vctFilePaths.end(); ++ite)
	{
		LPSHELLFOLDER psfFolderTemp = NULL;
		LPITEMIDLIST pidlItem = NULL;
		if (GetShellFolderAndRelativeItemidlist((*ite).c_str(), &psfFolderTemp, &pidlItem))
		{
			idList[nIDCount++] = pidlItem;

			if (psfFolder == NULL)
			{
				psfFolder = psfFolderTemp;
			}
			else
			{
				psfFolderTemp->Release(); 
			}
		}
	}

	BOOL bResult = FALSE;
	if (nIDCount > 0)
	{ 
		LPCONTEXTMENU pContextMenu = NULL;
		if (SUCCEEDED(psfFolder->GetUIObjectOf(hwnd, nIDCount, (LPCITEMIDLIST*)idList, IID_IContextMenu, NULL, (LPVOID*)&pContextMenu)))
		{
			bResult = PopContextMenu(hwnd, point, pContextMenu);
			pContextMenu->Release();
		}
		
		for (int i = 0; i < nIDCount; ++i)
		{
			PIDL::ILFree(idList[i]);
		}
		psfFolder->Release(); 
	}

	delete []idList;
	return bResult; 
}


BOOL OS::PopContextMenu(HWND hwnd, POINT point, LPCONTEXTMENU pContextMenu)
{
	BOOL bResult = FALSE;

	HMENU hMenu = CreatePopupMenu(); 
	HRESULT hr = pContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_EXPLORE);//CMF_ITEMMENU
	if (SUCCEEDED(hr))   
	{
#if 0
		int nCount = ::GetMenuItemCount(hMenu);
		for (int i = 0; i < nCount; ++i)
		{
			MENUITEMINFO info = {0};
			TCHAR szText[MAX_PATH] = {0};
			info.cbSize = sizeof(info);
			info.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE |MIIM_SUBMENU;
			info.dwTypeData = (LPTSTR)szText;
			info.cch = MAX_PATH;
			GetMenuItemInfo(hMenu, i, TRUE, &info);
			int err = GetLastError();

			if (info.hSubMenu)
			{
				int nCount2 = ::GetMenuItemCount(info.hSubMenu);
				for (int i2 = 0; i2 < nCount2; ++i2)
				{
					MENUITEMINFO info2 = {0};
					TCHAR szText2[MAX_PATH] = {0};
					info2.cbSize = sizeof(info2);
					info2.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE |MIIM_SUBMENU;
					info2.dwTypeData = (LPTSTR)szText2;
					info2.cch = MAX_PATH;
					GetMenuItemInfo(info.hSubMenu, i2, TRUE, &info2);
					int err = GetLastError();
				}

			}
		}
#endif
		::ClientToScreen(hwnd, &point);
		UINT nCmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, 0, hwnd, NULL); 
		if(nCmd)
		{
			CMINVOKECOMMANDINFO ici = {0};
			ici.cbSize = sizeof(ici); 
			ici.fMask = 0; 
			ici.hwnd = hwnd; 
			ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(nCmd - 1); 
			ici.lpParameters = NULL; 
			ici.lpDirectory = NULL; 
			ici.nShow = SW_SHOWNORMAL; 
			ici.dwHotKey = 0; 
			ici.hIcon = NULL; 

			if (SUCCEEDED(pContextMenu->InvokeCommand(&ici))) 
			{
				bResult = TRUE; 
			}
		} 
		::DestroyMenu(hMenu); 
	}

	return bResult;
}

BOOL OS::PopContextMenuEx(HWND hwnd, POINT point, LPCONTEXTMENU pContextMenu, LPCTSTR pszFilePath)
{
	BOOL bResult = FALSE;

	HMENU hMenu = CreatePopupMenu(); 
	HRESULT hr = pContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_EXPLORE);//CMF_ITEMMENU);
	if (SUCCEEDED(hr))   
	{
#if 0
		int nCount = ::GetMenuItemCount(hMenu);
		for (int i = 0; i < nCount; ++i)
		{
			MENUITEMINFO info = {0};
			TCHAR szText[MAX_PATH] = {0};
			info.cbSize = sizeof(info);
			info.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE;
			info.dwTypeData = (LPTSTR)szText;
			info.cch = MAX_PATH;
			GetMenuItemInfo(hMenu, i, TRUE, &info);
			int err = GetLastError();
		}
#endif

		::ClientToScreen(hwnd, &point);
		UINT nCmd = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, 0, hwnd, NULL);
		// 剪切复制菜单，直接InvokeCommand没有效果，原因未知。
		if (nCmd == 0x19)
		{
			return CutOrCopyFiles(pszFilePath, FALSE);
		}
		else if (nCmd == 0x1a)
		{
			return CutOrCopyFiles(pszFilePath, TRUE);
		}
		else if(nCmd)
		{
			CMINVOKECOMMANDINFO ici = {0};
			ici.cbSize = sizeof(ici); 
			ici.fMask = 0; 
			ici.hwnd = hwnd; 
			ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(nCmd - 1); 
			ici.lpParameters = NULL; 
			ici.lpDirectory = NULL; 
			ici.nShow = SW_SHOWNORMAL; 
			ici.dwHotKey = 0; 
			ici.hIcon = NULL; 

			if (SUCCEEDED(pContextMenu->InvokeCommand(&ici))) 
			{
				bResult = TRUE; 
			}
		} 
		::DestroyMenu(hMenu); 
	}

	return bResult;
}

BOOL OS::ShowSysFolerContextMenu(HWND hwnd, int nFolder, POINT point)
{
	BOOL bRet = FALSE;

	HRESULT   hr;	
	IShellFolder*   pDesktopFolder   =   NULL; 

	//   Get   desktop   folder. 
	hr = ::SHGetDesktopFolder(&pDesktopFolder); 

	ITEMIDLIST*   pidlFolder   =   NULL; 
	hr = ::SHGetFolderLocation(NULL, nFolder, NULL, 0, &pidlFolder); 

	if (SUCCEEDED(hr))
	{ 
		IContextMenu*   pContextMenu   =   NULL;
		const   ITEMIDLIST*   pidl2   =   pidlFolder; 
		hr   =   pDesktopFolder-> GetUIObjectOf(NULL, 1, &pidl2, IID_IContextMenu, NULL, (void**)&pContextMenu); 
		
		if (SUCCEEDED(hr))
		{
			bRet = PopContextMenu(hwnd, point, pContextMenu);
			pContextMenu-> Release(); 
		}

		PIDL::ILFree(pidlFolder);
	}
	pDesktopFolder->Release();
	return bRet;
}

BOOL OS::ShowSysContextMenu(HWND hwnd, wchar_t* Path, POINT point)
{
	BOOL bRet = FALSE;
	HRESULT   hr;
#if 0
	IShellFolder*   pDesktopFolder   =   NULL; 
	//   Get   desktop   folder. 
	hr = ::SHGetDesktopFolder(&pDesktopFolder); 
	ITEMIDLIST*   pidlFolder   =   NULL;
    ULONG attributes = SFGAO_FOLDER | SFGAO_LINK | SFGAO_CANMOVE | SFGAO_HASPROPSHEET | SFGAO_CANRENAME;
    hr = SHParseDisplayName(Path, NULL, &pidlFolder, attributes, &attributes);
#endif

	LPSHELLFOLDER psfFolder = NULL;
	LPITEMIDLIST pidlItem = NULL;
	if (GetShellFolderAndRelativeItemidlist(Path, &psfFolder, &pidlItem))
	{ 
		IContextMenu*   pContextMenu   =   NULL;
		const   ITEMIDLIST*   pidl2   =   pidlItem; 
		hr   =   psfFolder-> GetUIObjectOf(NULL, 1, &pidl2, IID_IContextMenu, NULL, (void**)&pContextMenu);
		if (SUCCEEDED(hr))
		{
			bRet = PopContextMenuEx(hwnd, point, pContextMenu, Path);
			pContextMenu-> Release(); 
		}

		PIDL::ILFree(pidlItem);
		psfFolder->Release();
	}
	
	return bRet;
}

HRESULT OS::SHInvokeCommand(HWND hwnd, IShellFolder* pShellFolder, LPCITEMIDLIST pidlItem, LPCSTR lpVerb)
{
    HRESULT hr = E_FAIL;
    if (pShellFolder)
    {
        IContextMenu *pContextMenu;
        if (SUCCEEDED(pShellFolder->GetUIObjectOf(hwnd, 1, &pidlItem, IID_IContextMenu, 0, (void**)&pContextMenu)))
        {
            if (pContextMenu)
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    if (SUCCEEDED(pContextMenu->QueryContextMenu(hmenu, 0, 1, 0x7FFF, lpVerb ? 0 : CMF_DEFAULTONLY))) 
                    {
                        UINT idCmd = -1;
                        if (lpVerb == NULL)
                        {
                            idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                            if ((UINT)-1 != idCmd)
                                lpVerb = MAKEINTRESOURCEA(idCmd - 1);
                        }

                        // if idCmd == 0, then lpVerb would be Zero. So we need to check to
                        // see if idCmd is not -1.
                        if (lpVerb || idCmd != (UINT)-1)
                        {
                            CMINVOKECOMMANDINFO ici = { 0 };
                            ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
                            ici.hwnd = hwnd;
                            ici.lpVerb = lpVerb;
                            ici.nShow = SW_NORMAL;

                            hr = pContextMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
                        }
                    }
                    DestroyMenu(hmenu);
                }
                // Release our use of the context menu
                pContextMenu->Release();
            }
        }
    }
    return hr;
}

HRESULT OS::ShellExecute(LPCWSTR FilePath)
{
    HRESULT hr = E_FAIL;
    if (FilePath != NULL)
    {
		LPSHELLFOLDER psfFolder = NULL;
		LPITEMIDLIST pidlItem = NULL;
		if (GetShellFolderAndRelativeItemidlist(FilePath, &psfFolder, &pidlItem))
		{          
            hr = SHInvokeCommand(::GetDesktopWindow(), psfFolder, pidlItem, NULL);
            psfFolder->Release();
            PIDL::ILFree(pidlItem);
        }
    }

    return hr;    
}

HICON OS::GetIcon(LPCITEMIDLIST pidl)
{
    HICON hIcon = NULL;
    SHFILEINFO shFi; 
    memset(&shFi, 0, sizeof(SHFILEINFO));
    if(SHGetFileInfoW((LPWSTR)pidl, 0, &shFi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_PIDL) != 0)              
    {
        IImageList *pIImageList = NULL;
        HRESULT hr = SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void **)&pIImageList);
        if (SUCCEEDED(hr))
        {
            pIImageList->GetIcon(shFi.iIcon, ILD_NORMAL, &hIcon);
            pIImageList->Release();
        }
    }

    return hIcon;
}

HICON OS::LoadIconFromShell(LPCTSTR FilePath)
{
    HICON hIcon = NULL;
    ULONG attributes = SFGAO_FOLDER | SFGAO_LINK | SFGAO_CANMOVE | SFGAO_HASPROPSHEET | SFGAO_CANRENAME;
    ULONG oldAttributes = attributes;
    LPITEMIDLIST ItemIdList = NULL;
    HRESULT hr = SHParseDisplayName(FilePath, NULL, &ItemIdList, attributes, &attributes);
    if (SUCCEEDED(hr))
    {
        if ((attributes & oldAttributes) != 0)
        {
            hIcon = GetIcon(ItemIdList);
        }

        PIDL::ILFree(ItemIdList);
    }

    return hIcon;
}

BOOL OS::CutOrCopyFiles(LPCWSTR pszFilePath, BOOL bCopy)
{
	UINT uLens = pszFilePath ? _tcslen(pszFilePath) : 0;
	if (uLens == 0)
	{
		return FALSE;
	}

	DROPFILES dropFiles = {0};
	UINT uGblLen,uDropFilesLen;
	HGLOBAL hGblFiles,hGblEffect;
	char *szData;

	DWORD *pdwDropEffect;

	const UINT uDropEffect=RegisterClipboardFormat(_T("Preferred DropEffect"));

	hGblEffect=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DWORD));

	pdwDropEffect=(DWORD*)GlobalLock(hGblEffect);
	*pdwDropEffect= (bCopy ? DROPEFFECT_COPY : DROPEFFECT_MOVE);
	GlobalUnlock(hGblEffect);

	uDropFilesLen=sizeof(dropFiles);
	dropFiles.pFiles =uDropFilesLen;
	dropFiles.pt.x=0;
	dropFiles.pt.y=0;
	dropFiles.fNC =FALSE;
	dropFiles.fWide =TRUE;

	uGblLen=uDropFilesLen + (uLens+1)*sizeof(WCHAR)+ 8;
	hGblFiles= GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, uGblLen);

	szData=(char*)GlobalLock(hGblFiles);
	memcpy(szData,(LPVOID)(&dropFiles),uDropFilesLen);

	wcscpy_s((WCHAR*)(szData+uDropFilesLen), uLens + 1, pszFilePath);
	GlobalUnlock(hGblFiles);

	if(OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_HDROP, hGblFiles);
		SetClipboardData(uDropEffect,hGblEffect);
		CloseClipboard();

		return TRUE;
	}

	return FALSE;
}

BOOL OS::CutOrCopyFiles(const std::vector<std::wstring> &vctFiles, BOOL bCopy)
{
	UINT uLens = 0;
	std::vector<std::wstring>::const_iterator ite = vctFiles.begin();
	for (; ite != vctFiles.end(); ++ite)
	{
		uLens += ite->length() + 1;
	}

	if (uLens == 0)
	{
		return FALSE;
	}

	DROPFILES dropFiles = {0};
	UINT uGblLen,uDropFilesLen;
	HGLOBAL hGblFiles,hGblEffect;
	char *szData;

	DWORD *pdwDropEffect;

	const UINT uDropEffect=RegisterClipboardFormat(_T("Preferred DropEffect"));
	hGblEffect=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DWORD));
	pdwDropEffect=(DWORD*)GlobalLock(hGblEffect);
	*pdwDropEffect= (bCopy ? DROPEFFECT_COPY : DROPEFFECT_MOVE);
	GlobalUnlock(hGblEffect);

	uDropFilesLen=sizeof(dropFiles);
	dropFiles.pFiles =uDropFilesLen;
	dropFiles.pt.x=0;
	dropFiles.pt.y=0;
	dropFiles.fNC =FALSE;
	dropFiles.fWide =TRUE;

	uGblLen=uDropFilesLen + (uLens+1)*sizeof(WCHAR)+ 8;
	hGblFiles= GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, uGblLen);

	szData=(char*)GlobalLock(hGblFiles);
	memcpy(szData,(LPVOID)(&dropFiles),uDropFilesLen);

	ite = vctFiles.begin();
	WCHAR *pPos = (WCHAR*)(szData+uDropFilesLen);
	for (; ite != vctFiles.end(); ++ite)
	{		
		wcscpy_s(pPos, ite->length() + 1, ite->c_str());
		pPos += ite->length() + 1;
	}
	
	GlobalUnlock(hGblFiles);

	if(OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_HDROP, hGblFiles);
		SetClipboardData(uDropEffect,hGblEffect);
		CloseClipboard();

		return TRUE;
	}

	return FALSE;
}

BOOL OS::ReadShortcut(LPCWSTR lpwLnkFile, LPWSTR lpDescFile, int length)
{
    BOOL isSuccess = FALSE;
    IShellLink *pShellLink = NULL;

    CoInitialize(NULL);

    HRESULT hr = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, (void **)&pShellLink);
    if(SUCCEEDED(hr))
    {
        IPersistFile *ppf;
        hr = pShellLink->QueryInterface(IID_IPersistFile, (void **)&ppf);
        if(SUCCEEDED(hr))
        {
            hr = ppf->Load(lpwLnkFile, TRUE);
            if(SUCCEEDED(hr))
            {
                hr = pShellLink->GetPath(lpDescFile, length, NULL, 0);
                isSuccess = SUCCEEDED(hr);
            }
            ppf->Release ();
        }
        pShellLink->Release ();
    }
    CoUninitialize();

    return isSuccess;
}

BOOL OS::GetShellFolderAndRelativeItemidlist(LPCWSTR pwszPath, IShellFolder **ppFolder, ITEMIDLIST **ppItemlist)
{
	LPSHELLFOLDER psfFolder = NULL;
	if (!SUCCEEDED(SHGetDesktopFolder(&psfFolder)))
	{ 
		return FALSE; 
	}

	ULONG ulCount = 0;
	ULONG ulAttr = 0;
	LPITEMIDLIST pidlMain = NULL;
	do
	{
		if (FAILED(psfFolder->ParseDisplayName(NULL, NULL, (LPWSTR)pwszPath, &ulCount, &pidlMain, &ulAttr)) || (pidlMain == NULL))
		{
			break;
		}

		UINT nCount = PIDL::ILGetCount(pidlMain);
		if (nCount == 0)   
		{
			break;
		}

		LPITEMIDLIST pidlItem = pidlMain;
		while (--nCount)
		{ 
			LPITEMIDLIST pidlNextItem = PIDL::ILCloneFirst (pidlItem); 
			if(pidlNextItem == NULL)
			{ 
				break; 
			}

			LPSHELLFOLDER psfNextFolder = NULL; 
			// Bind   to   the   folder   specified   in   the   new   item   ID   list. 
			if(!SUCCEEDED(psfFolder->BindToObject(pidlNextItem, NULL, IID_IShellFolder, (void**)&psfNextFolder)))   
			{ 
				PIDL::ILFree(pidlNextItem); 
				break; 
			} 
			// Release   the   IShellFolder   pointer   to   the   parent   folder   and   set   psfFolder   equal   to   the   IShellFolder   pointer   for   the   current   folder. 
			psfFolder->Release(); 
			psfFolder = psfNextFolder; 
			PIDL::ILFree(pidlNextItem); 
			pidlItem = (LPITEMIDLIST)PIDL::ILGetNext(pidlItem);
		}

		if (nCount != 0)
		{// 上面的循环出错
			break;
		}

		*ppFolder = psfFolder;
		*ppItemlist = PIDL::ILClone(pidlItem);
		PIDL::ILFree(pidlMain);
		return TRUE;

	}while(0);

	psfFolder->Release();
	if (pidlMain)
	{
		PIDL::ILFree(pidlMain);
	}
	return FALSE;
}