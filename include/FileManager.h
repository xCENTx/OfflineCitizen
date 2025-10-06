#pragma once

namespace FileManagement
{
	class FileManager
	{
	public:
		FileManager() {
			// Initialize COM for using the Windows API
			CoInitialize(NULL);
		}

		~FileManager() {
			// Cleanup COM
			CoUninitialize();
		}

		//	file functions
		static bool										file_create(const std::string& filePath);
		static bool										file_delete(const std::string& filePath);
		static bool										file_exists(const std::string& filePath);
		static bool										file_rename(const std::string& filePath, const std::string& fileName);
		static bool										file_move(const std::string srcPath, const std::string dstPath);
		static bool										file_write(const std::string& path, const std::string& input, unsigned int szLength, DWORD* out);
		static bool										file_read(const std::string& path, const std::string& input, unsigned int szLength, DWORD* out);

		//	directory functions
		static bool										dir_create(const std::string& dirPath);
		static bool										dir_exists(const std::string& dirPath);
		static std::string								dir_current();

		//	path functions
		static std::string								getKnownFolderPath(const KNOWNFOLDERID& folderId);
		static std::string								getSpecialFolderPath(int csidl);
		static std::string								path_MyDocuments();
		static std::string								path_AppData();
		static std::string								path_AppDataLocal();
		static std::string								path_AppDataRoaming();
		static std::string								path_ProgramData();

		//	open file dialogue window
		static bool										wndw_getOpenFilePath(std::string* path, std::string extension = "");
		static bool										wndw_getSaveFilePath(std::string* path, std::string extension = "");
	};

	class CJSON : public FileManager
	{
	public:
		static	std::string								fDirName;

	public:
		static bool										load(const std::string& file, nlohmann::json* data);

	public:
		class UserManager
		{
		public:
			struct UserEntry
			{
				std::string								hwid;			//	Windows Identifier
				std::string								client;			//	SC Player ID
				std::string								name;			//	PlayerName
				long long int							key;			//	DiscordID
			};
		public:
			static std::string							fUserName;
			static std::string							fSettingName;
			static bool									loadUserData(UserEntry* p);

		private: 
			static bool LoadUserInfo(UserEntry* p);
			static bool LoadUserSettings(UserEntry* p);
			static bool LoadAppData(std::string* result);
		};

	};

	class StringHelper
	{
	public:
		static std::string								FormatString(const char* fmt, ...);
		static std::string								FormatDistance(const float& distance) noexcept; // "[10km]"
		// static Structs::FVector2D						CalcTextSize(const std::string& text, const float& szFont = 8.f) noexcept;	//	Calculates the size of a text
		static void										CopyToClipboard(const char* input, ...);
	};
}

