#include <pch.h>
#include "gui.h"	
#include <game.h>

/* */

const ImRect gui::GetOverlayRect()
{
    const ImVec2 posClone = m_dxWindow.GetCloneWindowPos();   //  get the position of the cloned application window
    const ImVec2 szClone = m_dxWindow.GetCloneWindowSize();   //  get the size of the cloned application window
    const ImVec2& halfClone = szClone * .5;                     //  half application window size

    //  Get Window Size
    ImVec2 szMenu(halfClone);           //  overlay imgui menu window size
    //  ImVec2 szMenuMax(800.f, 600.f);     //  max overlay imgui menu window size
    //  szMenu.x = (szMenu.x > szMenuMax.x) ? szMenuMax.x : szMenu.x;
    //  szMenu.y = (szMenu.y > szMenuMax.y) ? szMenuMax.y : szMenu.y;

    //  Get Window Position
    ImVec2 posMenu = posClone + halfClone - szMenu * .5;   //  overlay imgui menu window position
    return ImRect(posMenu, posMenu + szMenu);
}

const ImRect gui::GetCloneRect()
{
    return ImRect(
        m_dxWindow.GetCloneWindowPos(),
        m_dxWindow.GetCloneWindowPos() + m_dxWindow.GetCloneWindowSize()
    );
}

const ImRect gui::GetScreenRect()
{
    return ImRect(
        ImVec2(0, 0),
        ImVec2(m_dxWindow.GetScreenSize())
    );
}

/* */

bool gui::init()
{
    if (g_bAppWindow)
    {
#if _DEBUG
        printf("[!][gui::init] failed to initialize because an application window has already been created.\n");
#endif
        return false;
    }

    g_bShowMenu = true;                                 //  set to true so that user is always greeted with the menu upon initialization

    //  establish overlay elements
	m_overlay.bIsShown = g_bShowMenu;                   //  pass in the show menu flag
    m_overlay.Menu = std::bind(&gui::OVERLAY, this);    //  pass overlay function
	m_overlay.Shroud = std::bind(&gui::SHROUD, this);   //  pass shroud function
	m_overlay.Hud = std::bind(&gui::HUD, this);         //  pass hud function

	//  initialize directx window & dear imgui
	m_dxWindow.Init();

    g_bAppWindow = true;    //  set flag so that no other windows can be created with any gui instance until this has shutdown.
	
    return true;
}

void gui::shutdown()
{
	m_dxWindow.Shutdown();  //  shutdown directx window & dear imgui
    m_dxWindow = DxWindow(); // reset class vars
	m_overlay = DxWindow::SOverlay();//  reset overlay elements
	g_bAppWindow = false;   //  reset application window flag
	g_bShowMenu = false;    //  reset show menu flag
}

void gui::update(const HWND& mGameWndw)
{
    if (!mGameWndw)
		return; //  @todo: log error

	m_dxWindow.CloneUpdate(mGameWndw);  //  update clone window info
    m_dxWindow.Update(m_overlay);       //  update overlay elements

    /* update canvas size reference for thread related events */
    const auto& szClone = m_dxWindow.GetCloneWindowSize();
}

void gui::UpdateViewState(const HWND& mGameWndw, const bool& bState) 
{
    if (!mGameWndw)
		return; //  @todo: log error

	bool bChanged = m_overlay.bIsShown != bState;   //  check if state has changed
	m_overlay.bIsShown = bState;                    //  renderWndw overlay is shown state

	//  renderWndw window focus
    if (bChanged)
		bState ? m_dxWindow.SetWindowFocus(m_dxWindow.GetWindowHandle()) : m_dxWindow.SetWindowFocus(mGameWndw);
}

void gui::OVERLAY()
{
    ASSERT_IMGUI;

    const ImGuiWindowFlags& dwFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    const auto& rect = this->GetOverlayRect();

    ImGui::SetNextWindowPos(rect.Min);
    ImGui::SetNextWindowSize(rect.GetSize());
    if (ImGui::Begin(__("OfflineCitizen"), &g_bShowMenu, dwFlags))
    {

        StarCitizen::Gui::renderMenu();

    }
    ImGui::End();
}

void gui::SHROUD()
{
    ASSERT_IMGUI;

    const ImGuiWindowFlags& dwFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
    const ImRect& rect = this->GetScreenRect();
    ImGui::SetNextWindowPos(rect.Min);
    ImGui::SetNextWindowSize(rect.GetSize());
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    if (ImGui::Begin(__("##SHROUDWINDOW"), (bool*)true, dwFlags))
    {

        StarCitizen::Gui::renderWall();
    
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

void gui::HUD()
{
    ASSERT_IMGUI;

    const ImGuiWindowFlags& dwFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
    const ImRect& rect = this->GetCloneRect();
    ImGui::SetNextWindowPos(rect.Min);
    ImGui::SetNextWindowSize(rect.GetSize());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    if (ImGui::Begin(__("##HUDWINDOW"), (bool*)true, dwFlags))
    {

        StarCitizen::Gui::renderWndw();
    
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::End();
}

/* */

void gui::widget::TextCentered(const char* pText)
{
    ASSERT_IMGUI;

    ImVec2 textSize = ImGui::CalcTextSize(pText);
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float textPosX = ImGui::GetCursorPosX() + (availableWidth - textSize.x) * 0.5f;
    ImGui::SetCursorPosX(textPosX);
    ImGui::Text("%s", pText);
}

void gui::widget::TextCenteredWindow(const char* pText)
{
    ASSERT_IMGUI;

    ImVec2 textSize = ImGui::CalcTextSize(pText);
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textPos = ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f);
    ImGui::SetCursorPos(textPos);
    ImGui::Text("%s", pText);
}

void gui::widget::Tooltip(const char* tip)
{
    ASSERT_IMGUI;

    if (!ImGui::IsItemHovered())
        return;

    ImGui::SetTooltip(tip);
}

void gui::widget::HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool gui::widget::BeginBlock(const char* pText, const ImVec2& size, const ImGuiChildFlags& dwFlags)
{
	ASSERT_IMGUI;

    /* get window size */
    ImVec2 szWindow = ImGui::GetContentRegionAvail();
    if (size.x != szWindow.x)
		szWindow.x = size.x;
	if (size.y != szWindow.y)
		szWindow.y = size.y;

    /* get flags */
    ImGuiChildFlags flags = ImGuiChildFlags_Border;
	if (dwFlags != 0)
		flags = dwFlags;

    /* render window & return result*/
    return ImGui::BeginChild( pText,  szWindow, flags, 0 );
}

void gui::widget::EndBlock()
{
    ASSERT_IMGUI;

    ImGui::EndChild();
}

bool gui::widget::ButtonWithToolTip(const char* pText, const char* pTip, const ImVec2& size)
{
    ASSERT_IMGUI;

	const bool& result = ImGui::Button(pText, size);
	gui::widget::Tooltip(pTip);
    return result;
}

bool gui::widget::CheckboxWithToolTip(const char* pText, const char* pTip, bool* pState)
{
    ASSERT_IMGUI;

	const bool& result = ImGui::Checkbox(pText, pState);
	gui::widget::Tooltip(pTip);
	return result;
}

bool gui::widget::SliderWithToolTip(const char* label, const char* tooltip, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
{
    ASSERT_IMGUI;

	const bool& result = ImGui::SliderScalar(label, data_type, p_data, p_min, p_max, format, flags);
	gui::widget::Tooltip(tooltip);
    return false;
}

/* */

void gui::draw::Text(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont)
{
    ASSERT_IMGUI;

    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), szFont, pos, color, text.c_str(), text.c_str() + text.size(), 800.f, nullptr);
}

void gui::draw::BGText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont)
{
    ASSERT_IMGUI;

    auto pFont = ImGui::GetFont();
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImRect textRect = ImRect(pos, pos + textSize);
    if (szFont > 0.f)
    {
        const ImVec2& scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
        ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
        textRect = (ImRect(scaledTextPos, scaledTextPos + scaledTextSize));
    }
    ImGui::GetWindowDrawList()->AddRectFilled(textRect.Min, textRect.Max, background);
    Text(textRect.Min, color, text, szFont);
}

void gui::draw::BorderText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont)
{
    ASSERT_IMGUI;

    auto pFont = ImGui::GetFont();
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImRect textRect = ImRect(pos, pos + textSize);
    if (szFont > 0.f)
    {
        const ImVec2& scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
        ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
        textRect = (ImRect(scaledTextPos, scaledTextPos + scaledTextSize));
    }
    ImGui::GetWindowDrawList()->AddRect(textRect.Min, textRect.Max, border);
    Text(textRect.Min, color, text, szFont);
}

void gui::draw::TextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont)
{
    ASSERT_IMGUI;

    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(pos.x - (textSize.x * 0.5f), pos.y);
    if (szFont <= 0.f)
    {
        Text(textPosition, color, text, szFont);
        return;
    }

    auto pFont = ImGui::GetFont();
    ImVec2 scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
    ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    Text(scaledTextPos, color, text, szFont);
}

void gui::draw::BGTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont)
{
    ASSERT_IMGUI;

    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(pos.x - (textSize.x * 0.5f), pos.y);
    if (szFont <= 0.f)
    {
        BGText(textPosition, color, text, background, szFont);
        return;
    }

    auto pFont = ImGui::GetFont();
    ImVec2 scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
    ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    ImGui::GetWindowDrawList()->AddRectFilled(scaledTextPos, scaledTextPos + scaledTextSize, background);
    Text(scaledTextPos, color, text, szFont);
}

void gui::draw::BorderTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont)
{
    ASSERT_IMGUI;

    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(pos.x - (textSize.x * 0.5f), pos.y);
    if (szFont <= 0.f)
    {
        BorderText(textPosition, color, text, border, szFont);
        return;
    }

    auto pFont = ImGui::GetFont();
    ImVec2 scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
    ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    ImGui::GetWindowDrawList()->AddRect(scaledTextPos, scaledTextPos + scaledTextSize, border);
    Text(scaledTextPos, color, text, szFont);
}

void gui::draw::Line(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness)
{
    ASSERT_IMGUI;

    ImGui::GetWindowDrawList()->AddLine(posA, posB, color, thickness);
}

void gui::draw::Circle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness, const float& segments)
{
    ASSERT_IMGUI;

    ImGui::GetWindowDrawList()->AddCircle(pos, radius, color, segments, thickness);
}

void gui::draw::CleanLine(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness)
{
    ASSERT_IMGUI;

    Line(posA, posB, ImColor(0.0f, 0.0f, 0.0f, color.Value.w), (thickness + 0.25));
    Line(posA, posB, ImColor(1.0f, 1.0f, 1.0f, color.Value.w), (thickness + 0.15));
    Line(posA, posB, color, thickness);
}

void gui::draw::CleanCircle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness, const float& segments)
{
    ASSERT_IMGUI;

    Circle(pos, ImColor(0.0f, 0.0f, 0.0f, color.Value.w), radius, thickness, segments);
    Circle(pos, ImColor(1.0f, 1.0f, 1.0f, color.Value.w), radius, thickness, segments);
    Circle(pos, color, radius, thickness, segments);
}

/* */

void gui::scene::Particles(const ImVec2& pos, const ImVec2& area, const ImColor& col)
{
    auto GetColorWithAlpha = [](ImColor color, float alpha)
        {
            return ImColor(color.Value.x, color.Value.y, color.Value.z, alpha);
        };

    static ImVec2 partile_pos[100];
    static ImVec2 partile_target_pos[100];
    static float partile_speed[100];
    static float partile_size[100];
    static float partile_radius[100];
    static float partile_rotate[100];

    auto io = ImGui::GetIO();
    ImDrawList* pDraw = ImGui::GetBackgroundDrawList();
    if (!pDraw)
        return;

    for (int i = 1; i < 100; i++)
    {
        if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
        {
            partile_pos[i].x = rand() % (int)area.x + 1;
            partile_pos[i].y = 15.f;
            partile_speed[i] = 1 + rand() % 25;
            partile_radius[i] = rand() % 4;
            partile_size[i] = rand() % 8;

            partile_target_pos[i].x = rand() % (int)area.x;
            partile_target_pos[i].y = area.y * 2;
        }

        partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], io.DeltaTime * (partile_speed[i] / 60));
        partile_rotate[i] += io.DeltaTime;

        if (partile_pos[i].y > area.y)
        {
            partile_pos[i].x = 0 + partile_size[i];
            partile_pos[i].y = 0 + partile_size[i];
            partile_rotate[i] = 0.f;
        }

        pDraw->AddCircleFilled(partile_pos[i] + pos, partile_size[i], GetColorWithAlpha(col, 0.5f), 60);
        pDraw->AddCircleFilled(partile_pos[i] + pos, partile_size[i] / 2, col, 60);
    }
}

/* */

ImRect gui::tools::CalcTextSize(const char* text, const float& szFont, ImVec2 pos)
{
	ASSERT_IMGUI;
    auto pFont = ImGui::GetFont();
    const ImVec2& textSize = ImGui::CalcTextSize(text);
    ImRect textRect = ImRect(pos, pos + textSize);

    if (szFont > 0.f)
    {
        const ImVec2& scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
        ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
        textRect = (ImRect(scaledTextPos, scaledTextPos + scaledTextSize));
    }

    return textRect;
}