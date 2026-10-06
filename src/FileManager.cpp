#include "pch.h"
#include "include/FileManager.h"

namespace FileManagement
{
	//---------------------------------------------------------------------------------------------------
	// 
	//	----------	[SECTION] FileManager
	//
	//---------------------------------------------------------------------------------------------------

#pragma region //	FileManager

	//---------------------------------------------------------------------------------------------------
	// Summary: Creates a new file at the specified file path.
	// Parameters:
	//   - filePath: The path of the file to be created.
	// Returns: `true` if the file creation is successful, `false` otherwise.
	bool FileManager::file_create(const std::string& filePath)
	{
		HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return true;
		}
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Deletes the file at the specified file path.
	// Parameters:
	//   - filePath: The path of the file to be deleted.
	// Returns: `true` if the file deletion is successful, `false` otherwise.
	bool FileManager::file_delete(const std::string& filePath)
	{
		return DeleteFileA(filePath.c_str());
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Checks if a file exists at the specified file path.
	// Parameters:
	//   - filePath: The path of the file to be checked.
	// Returns: `true` if the file exists, `false` otherwise.
	bool FileManager::file_exists(const std::string& filePath)
	{
		std::ifstream file(filePath);
		return file.good();
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Renames a file at the specified file path.
	// Parameters:
	//   - filePath: The path of the file to be renamed.
	//   - fileName: The new name for the file.
	// Returns: `true` if the file renaming is successful, `false` otherwise.
	bool FileManager::file_rename(const std::string& filePath, const std::string& fileName)
	{
		if (!file_exists(filePath))
			return false;


		// Extract directory path from file path
		size_t lastSlashIndex = filePath.find_last_of("/\\");
		if (lastSlashIndex == std::string::npos) {
			// File path does not contain directory separator
			return false;
		}
		std::string directoryPath = filePath.substr(0, lastSlashIndex);

		// Generate new file path with the new name
		std::string newFilePath = directoryPath + "\\" + fileName;

		// Rename the file
		if (rename(filePath.c_str(), newFilePath.c_str())) {
			return false;
		}

		return true;
	}

	//---------------------------------------------------------------------------------------------------
	bool FileManager::file_move(const std::string src, const std::string dst)
	{
		return MoveFileA(src.c_str(), dst.c_str());
	}

	//---------------------------------------------------------------------------------------------------
	bool FileManager::file_write(const std::string& p, const std::string& i, unsigned int length, DWORD* out)
	{
		DWORD temp = 0;
		if (out == NULL)
			out = &temp;
		*out = 0;
		auto handle = CreateFileA(p.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle)
		{
			bool result = WriteFile(handle, i.c_str(), length, out, NULL) && *out > 0;
			CloseHandle(handle);
			return result;
		}
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	bool FileManager::file_read(const std::string& p, const std::string& i, unsigned int length, DWORD* out)
	{
		DWORD temp = 0;
		if (out == NULL)
			out = &temp;
		*out = 0;
		auto handle = CreateFileA(p.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle)
		{
			bool result = ReadFile(handle, (LPVOID)i.c_str(), length, out, NULL) && *out > 0;
			CloseHandle(handle);
			return result;
		}
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Creates a new directory at the specified directory path.
	// Parameters:
	//   - dirPath: The path of the directory to be created.
	// Returns: `true` if the directory creation is successful, `false` otherwise.
	bool FileManager::dir_create(const std::string& dirPath)
	{
		return CreateDirectoryA(dirPath.c_str(), 0) || ERROR_ALREADY_EXISTS == GetLastError();
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Checks if a directory exists at the specified directory path.
	// Parameters:
	//   - dirPath: The path of the directory to be checked.
	// Returns: `true` if the directory exists, `false` otherwise.
	bool FileManager::dir_exists(const std::string& dirPath)
	{
		DWORD attr = GetFileAttributesA(dirPath.c_str());
		return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
	}

	//---------------------------------------------------------------------------------------------------
	std::string FileManager::dir_current()
	{
		char out[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, out);
		return std::string(out);
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to a known folder specified by its KNOWNFOLDERID.
	// Parameters:
	//   - folderId: The KNOWNFOLDERID of the folder whose path is to be retrieved.
	// Returns: The path to the specified known folder.
	std::string FileManager::getKnownFolderPath(const KNOWNFOLDERID& folderId)
	{
		PWSTR path;
		if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, 0, &path)))
		{
			std::wstring ws(path);
			CoTaskMemFree(path);
			return std::string(ws.begin(), ws.end());
		}
		return std::string();
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to a special folder specified by its CSIDL (Constant Special Item ID List) value.
	// Parameters:
	//   - csidl: The CSIDL value of the special folder whose path is to be retrieved.
	// Returns: The path to the specified special folder.
	std::string FileManager::getSpecialFolderPath(int csidl)
	{
		PWSTR path{};
		if (SUCCEEDED(SHGetFolderPath(0, csidl, 0, 0, path))) {

			std::wstring ws(path);
			CoTaskMemFree(path);
			return std::string(ws.begin(), ws.end());
		}
		return std::string();
	}

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to the My Documents folder.
	// Returns: The path to the My Documents folder.
	std::string FileManager::path_MyDocuments() { return getKnownFolderPath(FOLDERID_Documents); }

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to the AppData folder.
	// Returns: The path to the AppData folder.
	std::string FileManager::path_AppData() { return getSpecialFolderPath(CSIDL_COMMON_APPDATA); }

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to the Local AppData folder.
	// Returns: The path to the Local AppData folder.
	std::string FileManager::path_AppDataLocal() { return getKnownFolderPath(FOLDERID_LocalAppData); }

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to the Roaming AppData folder.
	// Returns: The path to the Roaming AppData folder.
	std::string FileManager::path_AppDataRoaming() { return getKnownFolderPath(FOLDERID_RoamingAppData); }

	//---------------------------------------------------------------------------------------------------
	// Summary: Retrieves the path to the ProgramData folder.
	// Returns: The path to the ProgramData folder.
	std::string FileManager::path_ProgramData() { return getKnownFolderPath(FOLDERID_ProgramData); }

	//---------------------------------------------------------------------------------------------------
	bool FileManager::wndw_getOpenFilePath(std::string* out, std::string extension)
	{
		bool result = false;

		//	CREATE FILE OBJECT INSTANCE
		HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(f_SysHr))
		{
			return result;
		}

		// CREATE FileOpenDialog OBJECT
		IFileOpenDialog* f_FileSystem;
		f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
		if (FAILED(f_SysHr))
		{
			CoUninitialize();
			return result;
		}

		///	SET FILE EXTENTION TO .DLL
		if (extension.size() > 0)
		{
			std::wstring res = std::wstring(extension.begin(), extension.end());
			f_SysHr = f_FileSystem->SetDefaultExtension(res.c_str());
			if (f_SysHr != S_OK)
			{
				CoUninitialize();
				return result;
			}
		}

		//	SHOW OPEN FILE DIALOG WINDOW
		f_SysHr = f_FileSystem->Show(NULL);
		if (f_SysHr != S_OK)
		{
			f_FileSystem->Release();
			CoUninitialize();
			return result;
		}

		//	RETRIEVE FILE NAME FROM THE SELECTED ITEM
		IShellItem* f_Files;
		f_SysHr = f_FileSystem->GetResult(&f_Files);
		if (f_SysHr != S_OK)
		{
			f_FileSystem->Release();
			CoUninitialize();
			return result;
		}

		//	STORE AND CONVERT THE FILE NAME
		PWSTR f_Path;
		f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
		if (f_SysHr != S_OK)
		{
			f_Files->Release();
			f_FileSystem->Release();
			CoUninitialize();
			return result;
		}

		//	set output name
		std::wstring path(f_Path);
		*out = std::string(path.begin(), path.end());

		//	SUCCESS, CLEAN UP
		CoTaskMemFree(f_Path);
		f_Files->Release();
		f_FileSystem->Release();
		CoUninitialize();
		return true;
	}

	//---------------------------------------------------------------------------------------------------
	bool FileManager::wndw_getSaveFilePath(std::string* out, std::string extension)
	{

		bool result{ false };

		//	Create File Object Instance
		HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(f_SysHr))
		{
			return result;
		}

		//	Create FileSaveDialog Object
		IFileSaveDialog* pSaveWndw;
		f_SysHr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSaveWndw));
		if (FAILED(f_SysHr))
		{
			CoUninitialize();
			return result;
		}

		//	Set File Extension
		if (extension.size() > 0)
		{
			std::wstring res = std::wstring(extension.begin(), extension.end());
			f_SysHr = pSaveWndw->SetDefaultExtension(res.c_str());
			if (f_SysHr != S_OK)
			{
				CoUninitialize();
				return result;
			}
		}

		//	Show Save File Window
		f_SysHr = pSaveWndw->Show(0);
		if (f_SysHr != S_OK)
		{
			pSaveWndw->Release();
			CoUninitialize();
			return result;
		}

		//	Retrieve input path & file name
		IShellItem* pFile;
		f_SysHr = pSaveWndw->GetResult(&pFile);
		if (f_SysHr != S_OK)
		{
			pSaveWndw->Release();
			CoUninitialize();
			return result;
		}

		//	get result path
		PWSTR pPath;
		f_SysHr = pFile->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
		if (f_SysHr != S_OK)
		{
			pFile->Release();
			pSaveWndw->Release();
			CoUninitialize();
			return result;
		}

		//	Store result & cleanup
		std::wstring wpath = pPath;
		*out = std::string(wpath.begin(), wpath.end());
		CoTaskMemFree(pPath);
		pFile->Release();
		pSaveWndw->Release();
		CoUninitialize();
		return true;
	}

#pragma endregion

	//---------------------------------------------------------------------------------------------------
	// 
	//	----------	[SECTION] CJSON
	//
	//---------------------------------------------------------------------------------------------------

#pragma region //	CJSON

	//---------------------------------------------------------------------------------------------------
	//	STATICS

	//---------------------------------------------------------------------------------------------------
	// Summary: Loads JSON data from a file into a `json` object.
	// Parameters:
	//   - file: The path to the file to load the data from.
	//   - data: A `json` object where the loaded data will be stored.
	// Returns: `true` if the data is successfully loaded, `false` otherwise.
	bool CJSON::load(const std::string& file, nlohmann::json* data)
	{
		bool result{ true };

		//	open file for reading
		std::ifstream jFile(file);
		if (!jFile.is_open())
			return result;

		//	obtain json data
		nlohmann::json jData;
		try
		{
			//	update contents
			jFile >> jData;

			//	pass data to pointer
			*data = jData;
		}
		catch (std::exception& e)
		{
			printf(xorstr_("[!][OfflineCitizen::SCJSON::load] failed to load json file '%s'.\n- MSG:\t'%s'\n"), file.c_str(), e.what());
			result ^= 1;
		}

		jFile.close();

		return result && data->size() > 0;
	}

#pragma endregion

}
