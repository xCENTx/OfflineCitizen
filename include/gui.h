#pragma once
#include <dxWindow.h>

inline int g_iMenuKey = VK_INSERT;		//	Menu key
inline bool g_bAppWindow = false;		//	Application window flag
inline bool g_bShowMenu = true;			//	Show the overlay window

#define ASSERT_IMGUI IM_ASSERT(ImGui::GetCurrentContext() != NULL);

#define IM_COL32_RED			IM_COL32(255,0,0,255)
#define IM_COL32_GREEN			IM_COL32(0,255,0,255)
#define IM_COL32_BLUE			IM_COL32(0,0,255,255)
#define IM_COL32_YELLOW			IM_COL32(255,255,0,255)
#define IM_COL32_CYAN			IM_COL32(0,255,255,255)

//	static canvas & widget rendering methods
class gui
{
public:
	//	Initializes the application window with directx11 and dear imgui
	//	@return bool - true if the initialization was successful, false otherwise
	bool init();

	//	Shuts down the application window and frees any resources
	void shutdown();

	//	Updates the application window and renders the widgets
	void update(const HWND& mGameWndw);

	//	update the overlay window state
	void UpdateViewState(const HWND& mGameWndw, const bool& bState);

public:
	//	@return ImRect - the overlay window bounds
	const ImRect GetOverlayRect();

	//	@return ImRect - the shroud window bounds
	const ImRect GetCloneRect();
	
	//	@return ImRect - the screen bounds
	const ImRect GetScreenRect();

public:
	struct widget
	{
	//	imgui window
		static void TextCentered(const char* pText);
		static void TextCenteredWindow(const char* pText);
		static void Tooltip(const char* tip);
		static void HelpMarker(const char* desc);

		/**/
		static bool BeginBlock(const char* pText, const ImVec2& size = ImVec2(0.f, 0.f), const ImGuiChildFlags& dwFlags = 0);
		static void EndBlock();

		/* tool tips */
		static void	TextWithToolTip(const char* pText, const char* pTip, ...);
		static bool ButtonWithToolTip(const char* pText, const char* pTip, const ImVec2& size = ImVec2(0.f, 0.f));
		static bool CheckboxWithToolTip(const char* pText, const char* pTip, bool* pState);
		static bool SliderWithToolTip(const char* pText, const char* pTip, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
		static bool	ComboWithToolTip(const char* label, const char* tip, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	};

	struct draw
	{
	//	canvas
		static void Text(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont = 0.f);
		static void BGText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont = 0.f);
		static void BorderText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont = 0.f);
		static void TextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont = 0.f);
		static void BGTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont = 0.f);
		static void BorderTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont = 0.f);
		static void Line(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness = 1.0f);
		static void Circle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness = 1.0f, const float& segments = 64);
		static void CleanLine(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness = 1.0f);
		static void CleanCircle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness = 1.0f, const float& segments = 64);
	};

	struct scene
	{
		static void Particles(const ImVec2& pos, const ImVec2& area, const ImColor& col);
	};

	struct tools
	{
		static ImRect CalcTextSize(const char* text, const float& szFont, ImVec2 pos = ImVec2(0.0f, 0.0f));
	};

private:
	//	Main overlay window
	void OVERLAY();

	//	Overlay background window
	void SHROUD();

	//	Click through overlay window
	void HUD();

	struct TABS
	{
		void Enhancements();
		void Dumper();
		void Config();
	};
	TABS tabs;

private:
	DxWindow m_dxWindow;			//	DirectX window
	DxWindow::SOverlay m_overlay;	//	Overlay Elements
};

