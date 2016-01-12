#ifndef __WIN32_SHELL_LINK_HPP
#define __WIN32_SHELL_LINK_HPP


#include <windows.h>
#include <shobjidl.h>
#include <ShlGuid.h>
#include <string>
#include <vector>

namespace OS
{
	// 创建快捷方式
	HRESULT CreateShortcut(LPCTSTR lpszPathLink, LPCTSTR lpszPathObj, LPCTSTR lpszDesc = NULL)
	{
		_com_ptr_t<uuid()> * psl = NULL;

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

	// 显示系统右键菜单
	// @pszPath 为文件或目录路径
	BOOL ShowContextMenu(HWND hwnd, LPCTSTR pszPath, POINT point);

	// 显示多个文件或目录的系统右键菜单
	// 多个文件或目录必须位于同一目录下
	BOOL ShowContextMenu(HWND hwnd, const std::vector<std::wstring> &vctFilePaths, POINT point);

	// 弹出系统右键菜单，但"剪切""复制"菜单响应无效，原因未知。
	BOOL PopContextMenu(HWND hwnd, POINT point, LPCONTEXTMENU pContextMenu);

	// 弹出系统右键菜单，函数自身处理"剪切""复制"点击。
	BOOL PopContextMenuEx(HWND hwnd, POINT point, LPCONTEXTMENU pContextMenu, LPCTSTR pszFilePath);

	// 显示系统文件夹的右键菜单，如我的电脑，回收站等
	// @nFolder A CSIDL value that identifies the folder，如CSIDL_DRIVES，CSIDL_BITBUCKET
	BOOL ShowSysFolerContextMenu(HWND hwnd, int nFolder, POINT point);

    BOOL ShowSysContextMenu(HWND hwnd, wchar_t* Path, POINT point);

    // 针对ITEMIDLIST调相应的命令
    //  lpVerb为空，则执行第一个默认命令
    HRESULT SHInvokeCommand(HWND hwnd, IShellFolder* pShellFolder, LPCITEMIDLIST pidlItem, LPCSTR lpVerb);

    //  通过右键菜单启动桌面程序
    HRESULT ShellExecute(LPCWSTR FilePath);

    HICON GetIcon(LPCITEMIDLIST pidl);

    HICON LoadIconFromShell(LPCWSTR FilePath);

	BOOL CutOrCopyFiles(LPCWSTR pszFilePath, BOOL bCopy);

	BOOL CutOrCopyFiles(const std::vector<std::wstring> &vctFiles, BOOL bCopy);

    BOOL ReadShortcut(LPCWSTR lpwLnkFile, LPWSTR lpDescFile, int length);

	BOOL GetShellFolderAndRelativeItemidlist(LPCWSTR pwszPath, IShellFolder **ppFolder, ITEMIDLIST **ppItemlist);
};


#endif