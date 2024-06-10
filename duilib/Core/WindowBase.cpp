#include "WindowBase.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowDropTarget.h"
#include "duilib/Utils/ApiWrapper.h"

#include <CommCtrl.h>
#include <Olectl.h>

namespace ui
{
WindowBase::WindowBase():
    m_hWnd(nullptr),
    m_hResModule(nullptr),
    m_pParentWindow(nullptr),
    m_bIsLayeredWindow(false),
    m_bFakeModal(false),
    m_bCloseing(false),
    m_hDcPaint(nullptr),
    m_bFullScreen(false),
    m_dwLastStyle(0),
    m_bUseSystemCaption(false),
    m_nWindowAlpha(255),
    m_bMouseCapture(false),
    m_pWindowDropTarget(nullptr),
    m_ptLastMousePos(-1, -1)
{
    m_rcLastWindowPlacement = { sizeof(WINDOWPLACEMENT), };
}

WindowBase::~WindowBase()
{
    ASSERT(m_hWnd == nullptr);
    ClearWindow();
}

bool WindowBase::CreateWnd(WindowBase* pParentWindow, const WindowCreateParam* pCreateParam, const UiRect& rc)
{
    WindowCreateParam createParam;
    if (pCreateParam != nullptr) {
        createParam = *pCreateParam;
    }
    ASSERT(!createParam.m_className.empty());
    if (createParam.m_className.empty()) {
        return false;
    }

    m_hResModule = createParam.m_hResModule;
    if (m_hResModule == nullptr) {
        m_hResModule = ::GetModuleHandle(nullptr);
    }

    //注册窗口类
    WNDCLASSEX wc = { 0 };
    wc.style = createParam.m_dwClassStyle;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.lpfnWndProc = WindowBase::__WndProc;
    wc.hInstance = GetResModuleHandle();
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = createParam.m_className.c_str();
    if (createParam.m_nClassLogoResId > 0) {
        wc.hIcon = LoadIcon(GetResModuleHandle(), (LPCTSTR)MAKEINTRESOURCE(createParam.m_nClassLogoResId));
        wc.hIconSm = LoadIcon(GetResModuleHandle(), (LPCTSTR)MAKEINTRESOURCE(createParam.m_nClassLogoResId));
    }
    else {
        wc.hIcon = nullptr;
        wc.hIconSm = nullptr;
    }
    ATOM ret = ::RegisterClassEx(&wc);
    bool bRet = (ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    ASSERT(bRet);
    if (!bRet) {
        return false;
    }

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (createParam.m_dwExStyle & WS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }
    m_pParentWindow = pParentWindow;
    m_parentFlag.reset();
    if (pParentWindow != nullptr) {
        m_parentFlag = pParentWindow->GetWeakFlag();
    }
    HWND hParentWnd = pParentWindow != nullptr ? pParentWindow->GetHWND() : nullptr;
    HWND hWnd = ::CreateWindowEx(createParam.m_dwExStyle,
                                 createParam.m_className.c_str(),
                                 createParam.m_windowTitle.c_str(),
                                 createParam.m_dwStyle,
                                 rc.left, rc.top, rc.Width(), rc.Height(),
                                 hParentWnd, NULL, GetResModuleHandle(), this);
    ASSERT(::IsWindow(hWnd));
    if (hWnd != m_hWnd) {
        m_hWnd = hWnd;
    }
    OnInitWindow();
    return hWnd != nullptr;
}

HMODULE WindowBase::GetResModuleHandle() const
{
    return (m_hResModule != nullptr) ? m_hResModule : (::GetModuleHandle(nullptr));
}

void WindowBase::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_bUseSystemCaption = bUseSystemCaption;
    OnUseSystemCaptionBarChanged();
}

bool WindowBase::IsUseSystemCaption() const
{
    return m_bUseSystemCaption;
}

void WindowBase::SetWindowAlpha(int nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nWindowAlpha = static_cast<uint8_t>(nAlpha);
    OnWindowAlphaChanged();
}

void WindowBase::SetLayeredWindow(bool bIsLayeredWindow)
{
    m_bIsLayeredWindow = bIsLayeredWindow;
    if (::IsWindow(m_hWnd)) {
        LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        if (m_bIsLayeredWindow) {
            dwExStyle |= WS_EX_LAYERED;
        }
        else {
            dwExStyle &= ~WS_EX_LAYERED;
        }
        ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
    }
    OnLayeredWindowChanged();
}

bool WindowBase::IsLayeredWindow() const
{
    return m_bIsLayeredWindow;
}

uint8_t WindowBase::GetWindowAlpha() const
{
    return m_nWindowAlpha;
}

void WindowBase::CloseWnd(UINT nRet)
{
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::PostMessage(m_hWnd, WM_CLOSE, (WPARAM)nRet, 0L);
}

void WindowBase::Close()
{
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::SendMessage(m_hWnd, WM_CLOSE, 0L, 0L);
}

LRESULT CALLBACK WindowBase::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowBase* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<WindowBase*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            pThis->m_hWnd = hWnd;
        }
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else {
        pThis = reinterpret_cast<WindowBase*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {
            LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
            ASSERT(hWnd == pThis->GetHWND());
            pThis->OnFinalMessage();
            return lRes;
        }
    }

    if (pThis != nullptr) {
        ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT WindowBase::CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

bool WindowBase::AddMessageFilter(IUIMessageFilter* pFilter)
{
    if (std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter) != m_aMessageFilters.end()) {
        ASSERT(false);
        return false;
    }
    if (pFilter != nullptr) {
        m_aMessageFilters.push_back(pFilter);
    }
    return true;
}
bool WindowBase::RemoveMessageFilter(IUIMessageFilter* pFilter)
{
    auto iter = std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter);
    if (iter != m_aMessageFilters.end()) {
        m_aMessageFilters.erase(iter);
        return true;
    }
    return false;
}

LRESULT WindowBase::SendMsg(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT WindowBase::PostMsg(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void WindowBase::PostQuitMsg(int32_t nExitCode)
{
    ::PostQuitMessage(nExitCode);
}

HDC WindowBase::GetPaintDC() const
{
    return m_hDcPaint;
}

void WindowBase::Invalidate(const UiRect& rcItem)
{
    GlobalManager::Instance().AssertUIThread();
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(m_hWnd, &rc, FALSE);
    // Invalidating a layered window will not trigger a WM_PAINT message,
    // thus we have to post WM_PAINT by ourselves.
    if (m_bIsLayeredWindow) {
        ::PostMessage(m_hWnd, WM_PAINT, 0, 0);
    }
}

LRESULT WindowBase::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = m_hWnd;
    if (uMsg == WM_CREATE) {
        //执行窗口的初始化工作        
        InitWindow();
    }

    //第一优先级：将消息发给过滤器进行过滤（可以通过设置bHandled为true来截获消息处理）
    for (auto filter : m_aMessageFilters) {
        bool bHandled = false;
        LRESULT lResult = filter->FilterMessage(uMsg, wParam, lParam, bHandled);
        if (bHandled) {
            //过滤器处理后截获此消息，不再进行派发
            return lResult;
        }
    }

    //第二优先级：派发给子类回调函数（子类可以通过设置bHandled为true来截获消息处理）
    bool bHandled = false;
    LRESULT lResult = 0;
    if (!bHandled) {
        lResult = OnWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    //第三优先级：内部处理的消息，处理后，不再派发
    if (!bHandled) {
        lResult = ProcessInternalMessage(uMsg, wParam, lParam, bHandled);
    }

    //第四优先级：内部处理函数，优先保证自身功能正常
    if (!bHandled) {
        lResult = ProcessWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    if (!bHandled && (uMsg == WM_CLOSE)) {
        //窗口即将关闭
        ClosingWindow();
        OnCloseWindow();
    }

    //自定义窗口消息的派发函数，仅供内部实现使用
    if (!bHandled && (uMsg >= WM_USER)) {
        lResult = HandleUserMessage(uMsg, wParam, lParam, bHandled);
    }

    //第五优先级：系统默认的窗口函数
    if (!bHandled && ::IsWindow(hWnd)) {
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lResult;
}

LRESULT WindowBase::OnWindowMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

WindowBase* WindowBase::WindowBaseFromPoint(const UiPoint& pt)
{
    WindowBase* pWindow = nullptr;
    HWND hWnd = ::WindowFromPoint({ pt.x, pt.y });
    if (::IsWindow(hWnd)) {
        if (hWnd == m_hWnd) {
            pWindow = this;
        }
        else {
            pWindow = reinterpret_cast<WindowBase*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if ((pWindow != nullptr) && (pWindow->m_hWnd != hWnd)) {
                pWindow = nullptr;
            }
        }
    }
    return pWindow;
}

HWND WindowBase::GetHWND() const
{
    return m_hWnd;
}

WindowBase* WindowBase::GetParentWindow() const
{
    if (!m_parentFlag.expired()) {
        return m_pParentWindow;
    }
    else {
        return nullptr;
    }
}

bool WindowBase::IsWindow() const
{
    return (m_hWnd != nullptr) && ::IsWindow(m_hWnd);
}

void WindowBase::InitWindow()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    ASSERT(m_hDcPaint == nullptr);
    if (m_hDcPaint != nullptr) {
        //避免重复初始化
        return;
    }

    //设置窗口风格（去除标题栏）
    HWND hWnd = GetHWND();
    uint32_t dwStyle = (uint32_t)::GetWindowLong(hWnd, GWL_STYLE);
    //使用自绘的标题栏：从原来窗口样式中，移除 WS_CAPTION 属性
    uint32_t dwNewStyle = dwStyle & ~WS_CAPTION;
    if (dwNewStyle != dwStyle) {
        ::SetWindowLong(hWnd, GWL_STYLE, dwNewStyle);
    }

    //创建绘制设备上下文
    m_hDcPaint = ::GetDC(hWnd);
    ASSERT(m_hDcPaint != nullptr);

    //注册接受Touch消息
    RegisterTouchWindowWrapper(hWnd, 0);

    //初始化窗口自身的DPI管理器
    const DpiManager& dpiManager = GlobalManager::Instance().Dpi();
    if (!dpiManager.IsUserDefineDpi() && dpiManager.IsPerMonitorDpiAware()) {
        //每个显示器，有独立的DPI：初始化窗口自己的DPI管理器
        m_dpi = std::make_unique<DpiManager>();
        m_dpi->SetDpiByWindow(this);
    }
}

void WindowBase::ClosingWindow()
{
    if (m_bFakeModal) {
        if (m_pParentWindow != nullptr) {
            m_pParentWindow->EnableWindow(true);
            m_pParentWindow->SetWindowFocus();
        }
        m_bFakeModal = false;
    }
    if (IsWindowFocused()) {
        SetOwnerWindowFocus();
    }
}

void WindowBase::ClearWindow()
{    
    //注销平板消息
    HWND hWnd = GetHWND();
    if (hWnd != nullptr) {
        UnregisterTouchWindowWrapper(hWnd);
    }

    //注销快捷键
    std::vector<int32_t> hotKeyIds = m_hotKeyIds;
    for (int32_t id : hotKeyIds) {
        UnregisterHotKey(id);
    }

    //注销拖放操作
    if (m_pWindowDropTarget != nullptr) {
        m_pWindowDropTarget->Clear();
        delete m_pWindowDropTarget;
        m_pWindowDropTarget = nullptr;
    }

    if (m_hDcPaint != nullptr) {
        ::ReleaseDC(m_hWnd, m_hDcPaint);
        m_hDcPaint = nullptr;
    }
    m_pParentWindow = nullptr;
    m_parentFlag.reset();
    m_dpi.reset();
    m_hWnd = nullptr;
}

void WindowBase::OnFinalMessage()
{
    ClearWindow();
    OnDeleteSelf();
}

void WindowBase::OnDeleteSelf()
{
    delete this;
}

void WindowBase::ScreenToClient(HWND hWnd, UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ScreenToClient(hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void WindowBase::ClientToScreen(HWND hWnd, UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ClientToScreen(hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void WindowBase::GetClientRect(HWND hWnd, UiRect& rcClient) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetClientRect(hWnd, &rc);
    rcClient = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void WindowBase::GetWindowRect(HWND hWnd, UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

bool WindowBase::GetMonitorWorkRect(HWND hWnd, UiRect& rcWork) const
{
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        return true;
    }
    else {
        ASSERT(!"WindowBase::GetMonitorWorkRect failed!");
        return false;
    }
}

bool WindowBase::GetMonitorRect(HWND hWnd, UiRect& rcMonitor, UiRect& rcWork) const
{
    rcMonitor.Clear();
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        rcMonitor = UiRect(oMonitor.rcMonitor.left, oMonitor.rcMonitor.top,
                        oMonitor.rcMonitor.right, oMonitor.rcMonitor.bottom);
        return true;
    }
    else {
        ASSERT(!"WindowBase::GetMonitorWorkRect failed!");
        return false;
    }
}

void WindowBase::MapWindowRect(HWND hwndFrom, HWND hwndTo, UiRect& rc) const
{
    POINT pts[2];
    pts[0].x = rc.left;
    pts[0].y = rc.top;
    pts[1].x = rc.right;
    pts[1].y = rc.bottom;
    ::MapWindowPoints((hwndFrom), (hwndTo), &pts[0], 2);
    rc.left = pts[0].x;
    rc.top = pts[0].y;
    rc.right = pts[1].x;
    rc.bottom = pts[1].y;
}

HWND WindowBase::GetWindowOwner() const
{
    return ::GetWindow(m_hWnd, GW_OWNER);
}

void WindowBase::GetClientRect(UiRect& rcClient) const
{
    GetClientRect(m_hWnd, rcClient);
}

void WindowBase::GetWindowRect(UiRect& rcWindow) const
{
    GetWindowRect(m_hWnd, rcWindow);
}

void WindowBase::ScreenToClient(UiPoint& pt) const
{
    ScreenToClient(m_hWnd, pt);
}

void WindowBase::ClientToScreen(UiPoint& pt) const
{
    ClientToScreen(m_hWnd, pt);
}

void WindowBase::GetCursorPos(UiPoint& pt) const
{
    POINT ptPos;
    ::GetCursorPos(&ptPos);
    pt = { ptPos.x, ptPos.y };
}

void WindowBase::MapWindowDesktopRect(UiRect& rc) const
{
    ASSERT(IsWindow());
    MapWindowRect(GetHWND(), HWND_DESKTOP, rc);
}

bool WindowBase::GetMonitorRect(UiRect& rcMonitor, UiRect& rcWork) const
{
    return GetMonitorRect(GetHWND(), rcMonitor, rcWork);
}

bool WindowBase::GetMonitorWorkRect(UiRect& rcWork) const
{
    return GetMonitorWorkRect(m_hWnd, rcWork);
}

bool WindowBase::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
{
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromPoint({ pt.x, pt.y }, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
            oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        return true;
    }
    else {
        ASSERT(!"WindowBase::GetMonitorWorkRect failed!");
        return false;
    }
}

void WindowBase::SetCapture()
{
    ::SetCapture(m_hWnd);
    m_bMouseCapture = true;
}

void WindowBase::ReleaseCapture()
{
    if (m_bMouseCapture) {
        ::ReleaseCapture();
        m_bMouseCapture = false;
    }
}

bool WindowBase::IsCaptured() const
{
    return m_bMouseCapture;
}

void WindowBase::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
    ASSERT(::IsWindow(m_hWnd));
    ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

void WindowBase::ShowModalFake()
{
    ASSERT(::IsWindow(m_hWnd));
    WindowBase* pParentWindow = m_pParentWindow;
    ASSERT((pParentWindow != nullptr) && (pParentWindow->GetHWND() != nullptr));
    if (pParentWindow != nullptr) {
        auto hOwnerWnd = GetWindowOwner();
        ASSERT(::IsWindow(hOwnerWnd));
        ASSERT_UNUSED_VARIABLE(hOwnerWnd == pParentWindow->GetHWND());
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow();
    m_bFakeModal = true;
}

bool WindowBase::IsFakeModal() const
{
    return m_bFakeModal;
}

void WindowBase::CenterWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD) == 0);
    UiRect rcDlg;
    GetWindowRect(rcDlg);
    UiRect rcArea;
    UiRect rcCenter;
    HWND hWnd = GetHWND();
    HWND hWndCenter = GetWindowOwner();
    if (hWndCenter != nullptr) {
        hWnd = hWndCenter;
    }

    // 处理多显示器模式下屏幕居中
    GetMonitorWorkRect(hWnd, rcArea);
    if (hWndCenter == nullptr) {
        rcCenter = rcArea;
    }
    else if (::IsIconic(hWndCenter)) {
        rcCenter = rcArea;
    }
    else {
        GetWindowRect(hWndCenter, rcCenter);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // The dialog is outside the screen, move it inside
    if (xLeft < rcArea.left) {
        xLeft = rcArea.left;
    }
    else if (xLeft + DlgWidth > rcArea.right) {
        xLeft = rcArea.right - DlgWidth;
    }
    if (yTop < rcArea.top) {
        yTop = rcArea.top;
    }
    else if (yTop + DlgHeight > rcArea.bottom) {
        yTop = rcArea.bottom - DlgHeight;
    }
    ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void WindowBase::ToTopMost()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void WindowBase::BringToTop()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void WindowBase::ActiveWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    if (IsWindowMinimized()) {
        ::ShowWindow(m_hWnd, SW_RESTORE);
    }
    else {
        if (!::IsWindowVisible(m_hWnd)) {
            ::ShowWindow(m_hWnd, SW_SHOW);
        }
        ::SetForegroundWindow(m_hWnd);
    }
}

bool WindowBase::Maximized()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    OnWindowMaximized();
    return true;
}

bool WindowBase::Restore()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    OnWindowRestored();
    return true;
}

bool WindowBase::Minimized()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    OnWindowMinimized();
    return true;
}

bool WindowBase::EnterFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (IsWindowMinimized()) {
        //最小化的时候，不允许激活全屏
        return false;
    }
    if (m_bFullScreen) {
        return true;
    }
    //保存窗口风格
    m_dwLastStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    //保存窗口大小位置信息
    m_rcLastWindowPlacement.length = sizeof(WINDOWPLACEMENT);
    ::GetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    int32_t xScreen = GetSystemMetricsForDpiWrapper(SM_XVIRTUALSCREEN, Dpi().GetDPI());
    int32_t yScreen = GetSystemMetricsForDpiWrapper(SM_YVIRTUALSCREEN, Dpi().GetDPI());
    int32_t cxScreen = GetSystemMetricsForDpiWrapper(SM_CXVIRTUALSCREEN, Dpi().GetDPI());
    int32_t cyScreen = GetSystemMetricsForDpiWrapper(SM_CYVIRTUALSCREEN, Dpi().GetDPI());

    // 去掉标题栏、边框
    DWORD dwFullScreenStyle = (m_dwLastStyle | WS_VISIBLE | WS_POPUP | WS_MAXIMIZE) & ~WS_CAPTION & ~WS_BORDER & ~WS_THICKFRAME & ~WS_DLGFRAME;
    ::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwFullScreenStyle);
    ::SetWindowPos(m_hWnd, NULL, xScreen, yScreen, cxScreen, cyScreen, SWP_FRAMECHANGED); // 设置位置和大小

    m_bFullScreen = true;
    OnWindowEnterFullScreen();
    return true;
}

bool WindowBase::ExitFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (!m_bFullScreen) {
        return false;
    }
    m_bFullScreen = false;

    //恢复窗口风格
    ::SetWindowLong(m_hWnd, GWL_STYLE, m_dwLastStyle);

    //恢复窗口位置/大小信息
    ::SetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    OnWindowExitFullScreen();

    if (IsWindowMaximized()) {
        Restore();
    }
    return true;
}

bool WindowBase::SetForeground()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetForegroundWindow() != m_hWnd) {
        ::SetForegroundWindow(m_hWnd);
    }
    return ::GetForegroundWindow() == m_hWnd;
}

bool WindowBase::SetWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
    return ::GetFocus() == m_hWnd;
}

bool WindowBase::KillWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() == m_hWnd) {
        ::SetFocus(nullptr);
    }
    return ::GetFocus() != m_hWnd;
}

bool WindowBase::IsWindowFocused() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetFocus());
}

bool WindowBase::SetOwnerWindowFocus()
{
    HWND hwndParent = GetWindowOwner();
    if (hwndParent != nullptr) {
        ::SetFocus(hwndParent);
        return ::GetFocus() == hwndParent;
    }
    return false;
}

void WindowBase::CheckSetWindowFocus()
{
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
}

bool WindowBase::IsWindowForeground() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetForegroundWindow());
}

bool WindowBase::IsWindowMaximized() const
{
    return ::IsWindow(m_hWnd) && ::IsZoomed(m_hWnd);
}

bool WindowBase::IsWindowMinimized() const
{
    return ::IsWindow(m_hWnd) && ::IsIconic(m_hWnd);
}

bool WindowBase::IsWindowFullScreen() const
{
    return m_bFullScreen;
}

bool WindowBase::UpdateWindow() const
{
    bool bRet = false;
    if ((m_hWnd != nullptr) && ::IsWindow(m_hWnd)) {
        bRet = ::UpdateWindow(m_hWnd) != FALSE;
    }
    return bRet;
}

bool WindowBase::IsZoomed() const
{
    return ::IsZoomed(GetHWND()) != FALSE;
}

bool WindowBase::IsIconic() const
{
    return ::IsIconic(GetHWND()) != FALSE;
}

bool WindowBase::EnableWindow(bool bEnable)
{
    return ::EnableWindow(GetHWND(), bEnable ? TRUE : false) != FALSE;
}

bool WindowBase::IsWindowEnabled() const
{
    return ::IsWindowEnabled(GetHWND()) != FALSE;
}

bool WindowBase::SetWindowPos(HWND hWndInsertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, UINT uFlags)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags) != FALSE;
}

bool WindowBase::SetWindowPos(const UiRect& rc, bool bNeedDpiScale, UINT uFlags, HWND hWndInsertAfter, bool bContainShadow)
{
    UiRect rcNewPos = rc;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(rcNewPos);
    }

    ASSERT(::IsWindow(m_hWnd));
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        rcNewPos.Inflate(rcShadow);
    }
    return SetWindowPos(hWndInsertAfter, rcNewPos.left, rcNewPos.top, rcNewPos.Width(), rcNewPos.Height(), uFlags);
}

bool WindowBase::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)
{
    ASSERT(IsWindow());
    return ::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint ? TRUE : FALSE) != FALSE;
}

UiRect WindowBase::GetWindowPos(bool bContainShadow) const
{
    ASSERT(IsWindow());
    UiRect rcPos;
    GetWindowRect(rcPos);
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        rcPos.Deflate(rcShadow);
    }
    return rcPos;
}

void WindowBase::Resize(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cy);
        Dpi().ScaleInt(cx);
    }

    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        cx += rcShadow.left + rcShadow.right;
        cy += rcShadow.top + rcShadow.bottom;
    }
    ASSERT(IsWindow());
    WindowBase::SetWindowPos(nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void WindowBase::SetIcon(UINT nRes)
{
    ASSERT(::IsWindow(m_hWnd));
    HICON hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                                     GetSystemMetricsForDpiWrapper(SM_CXICON, Dpi().GetDPI()),
                                     GetSystemMetricsForDpiWrapper(SM_CYICON, Dpi().GetDPI()),
                                     LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
    hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                               GetSystemMetricsForDpiWrapper(SM_CXSMICON, Dpi().GetDPI()),
                               GetSystemMetricsForDpiWrapper(SM_CYSMICON, Dpi().GetDPI()),
                               LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
}

void WindowBase::SetText(const DString& strText)
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowText(m_hWnd, strText.c_str());
    m_text = strText;
}

DString WindowBase::GetText() const
{
    return m_text;
}

void WindowBase::SetTextId(const DString& strTextId)
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowText(m_hWnd, GlobalManager::Instance().Lang().GetStringViaID(strTextId).c_str());
    m_textId = strTextId;
}

DString WindowBase::GetTextId() const
{
    return m_textId;
}

const DpiManager& WindowBase::Dpi() const
{
    return (m_dpi != nullptr) ? *m_dpi : GlobalManager::Instance().Dpi();
}

bool WindowBase::ChangeDpi(uint32_t nNewDPI)
{
    ASSERT(m_hWnd != nullptr);
    if (m_hWnd == nullptr) {
        return false;
    }
    //DPI值限制在60到300之间(小于50的时候，会出问题，比如原来是1的，经过DPI转换后，会变成0，导致很多逻辑失效)
    ASSERT((nNewDPI >= 60) && (nNewDPI <= 300)) ;
    if ((nNewDPI < 60) || (nNewDPI > 300)) {
        return false;
    }

    uint32_t nOldDPI = Dpi().GetDPI();
    uint32_t nOldDpiScale = Dpi().GetScale();
    if (m_dpi == nullptr) {
        m_dpi = std::make_unique<DpiManager>();
    }
    //更新窗口的DPI值为新值
    m_dpi->SetDPI(nNewDPI);

    ASSERT(nNewDPI == m_dpi->GetDPI());
    uint32_t nNewDpiScale = m_dpi->GetScale();

    //按新的DPI更新窗口布局
    OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);

    //更新窗口大小和位置
    UiRect rcOldWindow;
    GetWindowRect(rcOldWindow);
    UiRect rcNewWindow = Dpi().GetScaleRect(rcOldWindow, nOldDpiScale);
    ::SetWindowPos(m_hWnd,
                   NULL,
                   rcOldWindow.left,
                   rcOldWindow.top,
                   rcNewWindow.Width(),
                   rcNewWindow.Height(),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    OnWindowDpiChanged(nOldDPI, nNewDPI);
    return true;
}

void WindowBase::ProcessDpiChangedMsg(uint32_t nNewDPI, const UiRect& rcNewWindow)
{
    //此消息必须处理，否则窗口大小与界面的比例将失调
    const DpiManager& dpiManager = GlobalManager::Instance().Dpi();
    if ((m_dpi != nullptr) && dpiManager.IsPerMonitorDpiAware()) {
        //调整DPI值
        uint32_t nOldDPI = m_dpi->GetDPI();
        uint32_t nOldDpiScale = m_dpi->GetScale();

        //更新窗口的DPI值为新值
        m_dpi->SetDPI(nNewDPI);
        ASSERT(nNewDPI == m_dpi->GetDPI());
        uint32_t nNewDpiScale = m_dpi->GetScale();

        //按新的DPI更新窗口布局
        OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);

        //更新窗口的位置和大小
        if (!rcNewWindow.IsEmpty()) {
            ::SetWindowPos(m_hWnd, NULL,
                           rcNewWindow.left, rcNewWindow.top, rcNewWindow.Width(), rcNewWindow.Height(),
                           SWP_NOZORDER | SWP_NOACTIVATE);
        }
        OnWindowDpiChanged(nOldDPI, nNewDPI);
    }
}

void WindowBase::OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if ((nOldDpiScale == nNewDpiScale) || (nNewDpiScale == 0)) {
        return;
    }
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    m_szMinWindow = Dpi().GetScaleSize(m_szMinWindow, nOldDpiScale);
    m_szMaxWindow = Dpi().GetScaleSize(m_szMaxWindow, nOldDpiScale);
    m_rcSizeBox = Dpi().GetScaleRect(m_rcSizeBox, nOldDpiScale);
    m_szRoundCorner = Dpi().GetScaleSize(m_szRoundCorner, nOldDpiScale);
    m_rcCaption = Dpi().GetScaleRect(m_rcCaption, nOldDpiScale);
    m_ptLastMousePos = Dpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);
}

bool WindowBase::SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw)
{
    ASSERT((szRoundCorner.cx > 0) && (szRoundCorner.cy > 0));
    if ((szRoundCorner.cx <= 0) || (szRoundCorner.cy <= 0)) {
        return false;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
    int nRet = ::SetWindowRgn(GetHWND(), hRgn, bRedraw ? TRUE : FALSE);
    ::DeleteObject(hRgn);//TODO: 检查是否需要删除，按MSDN说法，是不需要删除的。
    return nRet != 0;
}

void WindowBase::ClearWindowRgn(bool bRedraw)
{
    ASSERT(IsWindow());
    ::SetWindowRgn(GetHWND(), nullptr, bRedraw ? TRUE : FALSE);
}

const UiRect& WindowBase::GetSizeBox() const
{
    return m_rcSizeBox;
}

void WindowBase::SetSizeBox(const UiRect& rcSizeBox, bool bNeedDpiScale)
{
    m_rcSizeBox = rcSizeBox;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcSizeBox);
    }
    if (m_rcSizeBox.left < 0) {
        m_rcSizeBox.left = 0;
    }
    if (m_rcSizeBox.top < 0) {
        m_rcSizeBox.top = 0;
    }
    if (m_rcSizeBox.right < 0) {
        m_rcSizeBox.right = 0;
    }
    if (m_rcSizeBox.bottom < 0) {
        m_rcSizeBox.bottom = 0;
    }
}

const UiRect& WindowBase::GetCaptionRect() const
{
    return m_rcCaption;
}

void WindowBase::SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale)
{
    m_rcCaption = rcCaption;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcCaption);
    }
}

const UiSize& WindowBase::GetRoundCorner() const
{
    return m_szRoundCorner;
}

void WindowBase::SetRoundCorner(int cx, int cy, bool bNeedDpiScale)
{
    ASSERT(cx >= 0);
    ASSERT(cy >= 0);
    if ((cx < 0) || (cy < 0)) {
        return;
    }
    //两个参数要么同时等于0，要么同时大于0，否则参数无效
    ASSERT(((cx > 0) && (cy > 0)) || ((cx == 0) && (cy == 0)));
    if (cx == 0) {
        if (cy != 0) {
            return;
        }
    }
    else {
        if (cy == 0) {
            return;
        }
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    m_szRoundCorner.cx = cx;
    m_szRoundCorner.cy = cy;
}

void WindowBase::OnUseSystemCaptionBarChanged()
{
    if (IsUseSystemCaption()) {
        //使用系统默认标题栏, 需要增加标题栏风格
        if (::IsWindow(GetHWND())) {
            UINT oldStyleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
            UINT newStyleValue = oldStyleValue | WS_CAPTION;
            if (newStyleValue != oldStyleValue) {
                ::SetWindowLong(GetHWND(), GWL_STYLE, newStyleValue);
            }
        }
        //关闭层窗口
        SetLayeredWindow(false);
    }
}

UiSize WindowBase::GetMinInfo(bool bContainShadow) const
{
    UiSize xy = m_szMinWindow;
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (xy.cx != 0) {
            xy.cx -= rcShadow.left + rcShadow.right;
        }
        if (xy.cy != 0) {
            xy.cy -= rcShadow.top + rcShadow.bottom;
        }
    }
    return xy;
}

void WindowBase::SetMinInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (cx != 0) {
            cx += rcShadow.left + rcShadow.right;
        }
        if (cy != 0) {
            cy += rcShadow.top + rcShadow.bottom;
        }
    }
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

UiSize WindowBase::GetMaxInfo(bool bContainShadow) const
{
    UiSize xy = m_szMaxWindow;
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (xy.cx != 0) {
            xy.cx -= rcShadow.left + rcShadow.right;
        }
        if (xy.cy != 0) {
            xy.cy -= rcShadow.top + rcShadow.bottom;
        }
    }

    return xy;
}

void WindowBase::SetMaxInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (cx != 0) {
            cx += rcShadow.left + rcShadow.right;
        }
        if (cy != 0) {
            cy += rcShadow.top + rcShadow.bottom;
        }
    }
    m_szMaxWindow.cx = cx;
    m_szMaxWindow.cy = cy;
}

int32_t WindowBase::SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    ASSERT(::IsWindow(GetHWND()));
    return (int32_t)::SendMessage(GetHWND(), WM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0);
}

bool WindowBase::GetWindowHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    ASSERT(::IsWindow(GetHWND()));
    DWORD dw = (DWORD)::SendMessage(GetHWND(), HKM_GETHOTKEY, 0, 0L);
    wVirtualKeyCode = LOBYTE(LOWORD(dw));
    wModifiers = HIBYTE(LOWORD(dw));
    return dw != 0;
}

bool WindowBase::RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)
{
    ASSERT(::IsWindow(GetHWND()));
    if (wVirtualKeyCode != 0) {
        UINT fsModifiers = 0;
        if (wModifiers & HOTKEYF_ALT)     fsModifiers |= MOD_ALT;
        if (wModifiers & HOTKEYF_CONTROL) fsModifiers |= MOD_CONTROL;
        if (wModifiers & HOTKEYF_SHIFT)   fsModifiers |= MOD_SHIFT;
        if (wModifiers & HOTKEYF_EXT)     fsModifiers |= MOD_WIN;

#ifndef MOD_NOREPEAT
        if (::IsWindows7OrGreater()) {
            fsModifiers |= 0x4000;
        }
#else
        fsModifiers |= MOD_NOREPEAT;
#endif

        LRESULT lResult = ::RegisterHotKey(this->GetHWND(), id, fsModifiers, wVirtualKeyCode);
        ASSERT(lResult != 0);
        if (lResult != 0) {
            auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
            if (iter != m_hotKeyIds.end()) {
                m_hotKeyIds.erase(iter);
            }
            m_hotKeyIds.push_back(id);
            return true;
        }
    }
    return false;
}

bool WindowBase::UnregisterHotKey(int32_t id)
{
    ASSERT(::IsWindow(GetHWND()));
    auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
    if (iter != m_hotKeyIds.end()) {
        m_hotKeyIds.erase(iter);
    }
    return ::UnregisterHotKey(GetHWND(), id);
}

bool WindowBase::RegisterDragDrop(ControlDropTarget* pDropTarget)
{
    if (m_pWindowDropTarget == nullptr) {
        m_pWindowDropTarget = new WindowDropTarget;
        m_pWindowDropTarget->SetWindow(this);
    }
    return m_pWindowDropTarget->RegisterDragDrop(pDropTarget);
}

bool WindowBase::UnregisterDragDrop(ControlDropTarget* pDropTarget)
{
    if (m_pWindowDropTarget == nullptr) {
        return false;
    }
    return m_pWindowDropTarget->UnregisterDragDrop(pDropTarget);
}

const UiPoint& WindowBase::GetLastMousePos() const
{
    return m_ptLastMousePos;
}

void WindowBase::SetLastMousePos(const UiPoint& pt)
{
    m_ptLastMousePos = pt;
}

LRESULT WindowBase::ProcessInternalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    bool bInternalMsg = true;
    switch (uMsg)
    {
    case WM_NCACTIVATE:         lResult = OnNcActivateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:         lResult = OnNcCalcSizeMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:          lResult = OnNcHitTestMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCLBUTTONDBLCLK:    lResult = OnNcLButtonDbClickMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_GETMINMAXINFO:      lResult = OnGetMinMaxInfoMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_ERASEBKGND:         lResult = OnEraseBkGndMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_DPICHANGED:         lResult = OnDpiChangedMsg(uMsg, wParam, lParam, bHandled); break;    
    case WM_WINDOWPOSCHANGING:  lResult = OnWindowPosChangingMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_NOTIFY:             lResult = OnNotifyMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_COMMAND:            lResult = OnCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLOREDIT:       lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLORSTATIC:     lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_SYSCOMMAND:         lResult = OnSysCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_TOUCH:              lResult = OnTouchMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_POINTERDOWN:
    case WM_POINTERUP:
    case WM_POINTERUPDATE:
    case WM_POINTERLEAVE:
    case WM_POINTERWHEEL:
    case WM_POINTERCAPTURECHANGED:
        lResult = OnPointerMsgs(uMsg, wParam, lParam, bHandled);
        break;

    default:
        bInternalMsg = false;
        break;
    }//end of switch

    if (bInternalMsg && !bHandled) {
        //调用窗口函数，然后不再继续传递此消息
        //bHandled = true;
        //lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lResult;
}

LRESULT WindowBase::OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCACTIVATE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    LRESULT lResult = 0;
    if (IsWindowMinimized()) {
        bHandled = false;
    }
    else {
        //MSDN: wParam 参数为 FALSE 时，应用程序应返回 TRUE 以指示系统应继续执行默认处理
        bHandled = true;
        lResult = (wParam == FALSE) ? TRUE : FALSE;
    }
    return lResult;
}

LRESULT WindowBase::OnNcCalcSizeMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCCALCSIZE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    //截获，让系统不处理此消息
    bHandled = true;
    return 0;
}

LRESULT WindowBase::OnNcHitTestMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCHITTEST);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    ScreenToClient(pt);

    UiRect rcClient;
    GetClientRect(rcClient);

    //客户区域，排除掉阴影部分区域
    UiPadding rcCorner;
    GetShadowCorner(rcCorner);
    rcClient.Deflate(rcCorner);

    if (!IsWindowMaximized()) {
        //非最大化状态
        UiRect rcSizeBox = GetSizeBox();
        if (pt.y < rcClient.top + rcSizeBox.top) {
            if (pt.y >= rcClient.top) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTTOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTTOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return HTTOP;//在窗口的上水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTBOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTBOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return HTBOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return HTLEFT;//在窗口的左边框
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return HTRIGHT;//在窗口的右边框中
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
    }

    UiRect rcCaption = GetCaptionRect();
    if ( (pt.x >= rcClient.left + rcCaption.left) && (pt.x < rcClient.right - rcCaption.right) &&
         (pt.y >= rcClient.top + rcCaption.top && pt.y < rcClient.top + rcCaption.bottom)) {
        if (IsPtInCaptionBarControl(pt)) {
            return HTCLIENT;//在工作区中（放在标题栏上的控件，视为工作区）
        }
        else {
            return HTCAPTION;//在标题栏中
        }
    }
    //其他，在工作区中
    return HTCLIENT;
}

LRESULT WindowBase::OnNcLButtonDbClickMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCLBUTTONDBLCLK);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    if (!IsWindowMaximized()) {
        //最大化
        Maximized();
    }
    else {
        //还原
        Restore();
    }
    return 0;
}

LRESULT WindowBase::OnEraseBkGndMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_ERASEBKGND);
    bHandled = true;
    return 1;
}

LRESULT WindowBase::OnGetMinMaxInfoMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_GETMINMAXINFO);
    bHandled = false;
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    UiRect rcWork;
    UiRect rcMonitor;
    GetMonitorRect(rcMonitor, rcWork);
    rcWork.Offset(-rcMonitor.left, -rcMonitor.top);

    //最大化时，默认设置为当前屏幕的最大区域
    lpMMI->ptMaxPosition.x = rcWork.left;
    lpMMI->ptMaxPosition.y = rcWork.top;
    lpMMI->ptMaxSize.x = rcWork.Width();
    lpMMI->ptMaxSize.y = rcWork.Height();

    if (GetMaxInfo(false).cx != 0) {
        lpMMI->ptMaxTrackSize.x = GetMaxInfo(true).cx;
    }
    if (GetMaxInfo(false).cy != 0) {
        lpMMI->ptMaxTrackSize.y = GetMaxInfo(true).cy;
    }
    if (GetMinInfo(false).cx != 0) {
        lpMMI->ptMinTrackSize.x = GetMinInfo(true).cx;
    }
    if (GetMinInfo(false).cy != 0) {
        lpMMI->ptMinTrackSize.y = GetMinInfo(true).cy;
    }
    return 0;
}

LRESULT WindowBase::OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_DPICHANGED);
    bHandled = false;//需要重新测试

    uint32_t nNewDPI = HIWORD(wParam);
    UiRect rcNewWindow;
    const RECT* prcNewWindow = (RECT*)lParam;
    if (prcNewWindow != nullptr) {
        rcNewWindow.left = prcNewWindow->left;
        rcNewWindow.top = prcNewWindow->top;
        rcNewWindow.right = prcNewWindow->right;
        rcNewWindow.bottom = prcNewWindow->bottom;
    }
    ProcessDpiChangedMsg(nNewDPI, rcNewWindow);
    return 0;
}

LRESULT WindowBase::OnWindowPosChangingMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_WINDOWPOSCHANGING);
    bHandled = false;
    if (IsWindowMaximized()) {
        //最大化状态
        LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
        if (lpPos->flags & SWP_FRAMECHANGED) // 第一次最大化，而不是最大化之后所触发的WINDOWPOSCHANGE
        {
            POINT pt = { 0, 0 };
            HMONITOR hMontorPrimary = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            HMONITOR hMonitorTo = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);

            // 先把窗口最大化，再最小化，然后恢复，此时MonitorFromWindow拿到的HMONITOR不准确
            // 判断GetWindowRect的位置如果不正确（最小化时得到的位置信息是-38000），则改用normal状态下的位置，来获取HMONITOR
            UiRect rc;
            GetWindowRect(rc);
            if (rc.left < -10000 && rc.top < -10000 && rc.bottom < -10000 && rc.right < -10000) {
                WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
                ::GetWindowPlacement(m_hWnd, &wp);
                hMonitorTo = ::MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);
            }
            if (hMonitorTo != hMontorPrimary) {
                // 解决无边框窗口在双屏下面（副屏分辨率大于主屏）时，最大化不正确的问题
                MONITORINFO  miTo = { sizeof(miTo), 0 };
                ::GetMonitorInfo(hMonitorTo, &miTo);

                lpPos->x = miTo.rcWork.left;
                lpPos->y = miTo.rcWork.top;
                lpPos->cx = miTo.rcWork.right - miTo.rcWork.left;
                lpPos->cy = miTo.rcWork.bottom - miTo.rcWork.top;
            }
        }
    }
    return 0;
}

LRESULT WindowBase::OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NOTIFY);
    bHandled = false;
    LPNMHDR lpNMHDR = (LPNMHDR)lParam;
    if (lpNMHDR != nullptr) {
        bHandled = true;
        return ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT WindowBase::OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_COMMAND);
    bHandled = false;
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT WindowBase::OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC);
    bHandled = false;
    // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
    // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT WindowBase::OnSysCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SYSCOMMAND);
    bHandled = true;
    if (GET_SC_WPARAM(wParam) == SC_CLOSE) {
        //立即关闭窗口
        Close();
        return 0;
    }
    //首先调用默认的窗口函数，使得命令生效
    bool bZoomed = IsZoomed();
    LRESULT lRes = CallDefaultWindowProc(uMsg, wParam, lParam);
    if (IsZoomed() != bZoomed) {
        if (GET_SC_WPARAM(wParam) == 0xF012) {
            //修复窗口最大化和还原按钮的状态（当在最大化时，向下拖动标题栏，窗口会改变为非最大化状态）
            OnWindowRestored();
        }
    }
    return lRes;
}

LRESULT WindowBase::OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_TOUCH);
    LRESULT lResult = 0;
    bHandled = false;
    unsigned int nNumInputs = LOWORD(wParam);
    if (nNumInputs < 1) {
        nNumInputs = 1;
    }
    TOUCHINPUT* pInputs = new TOUCHINPUT[nNumInputs];
    // 只关心第一个触摸位置
    if (!GetTouchInputInfoWrapper((HTOUCHINPUT)lParam, nNumInputs, pInputs, sizeof(TOUCHINPUT))) {
        delete[] pInputs;
        return lResult;
    }
    else {
        CloseTouchInputHandleWrapper((HTOUCHINPUT)lParam);
        if (pInputs[0].dwID == 0) {
            return lResult;
        }
    }
    //获取触摸点的坐标，并转换为窗口内的客户区坐标
    UiPoint pt = { TOUCH_COORD_TO_PIXEL(pInputs[0].x) , TOUCH_COORD_TO_PIXEL(pInputs[0].y) };
    ScreenToClient(pt);

    DWORD dwFlags = pInputs[0].dwFlags;
    delete[] pInputs;
    pInputs = nullptr;

    if (dwFlags & TOUCHEVENTF_DOWN) {
        lResult = OnMouseLButtonDownMsg(pt, 0, bHandled);
    }
    else if (dwFlags & TOUCHEVENTF_MOVE) {
        UiPoint lastMousePos = m_ptLastMousePos;
        lResult = OnMouseMoveMsg(pt, 0, bHandled);
        int detaValue = pt.y - lastMousePos.y;
        if (detaValue != 0) {
            //触发滚轮功能（lParam参数故意设置为0，有特殊含义）
            lResult = OnMouseWheelMsg(detaValue, pt, 0, bHandled);
        }
    }
    else if (dwFlags & TOUCHEVENTF_UP) {
        lResult = OnMouseLButtonUpMsg(pt, 0, bHandled);
    }
    return lResult;
}

LRESULT WindowBase::OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_POINTERDOWN ||
                           uMsg == WM_POINTERUP ||
                           uMsg == WM_POINTERUPDATE ||
                           uMsg == WM_POINTERLEAVE ||
                           uMsg == WM_POINTERCAPTURECHANGED ||
                           uMsg == WM_POINTERWHEEL);

    LRESULT lResult = 0;
    bHandled = false;
    // 只关心第一个触摸点
    if (!IS_POINTER_PRIMARY_WPARAM(wParam)) {
        bHandled = true;
        return lResult;
    }
    //获取指针位置，并且将屏幕坐标转换为窗口客户区坐标
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    ScreenToClient(pt);
    switch (uMsg)
    {
    case WM_POINTERDOWN:
        lResult = OnMouseLButtonDownMsg(pt, 0, bHandled);
        bHandled = true;
        break;
    case WM_POINTERUPDATE:
        lResult = OnMouseMoveMsg(pt, 0, bHandled);
        bHandled = true;
        break;
    case WM_POINTERUP:
        lResult = OnMouseLButtonUpMsg(pt, 0, bHandled);
        bHandled = true;
        break;
    case WM_POINTERWHEEL:
        {
            int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            lResult = OnMouseWheelMsg(wheelDelta, pt, 0, bHandled);
            bHandled = true;
        }
        break;
    case WM_POINTERLEAVE:
        lResult = OnMouseLeaveMsg(pt, 0, bHandled);
        break;
    case WM_POINTERCAPTURECHANGED:
        lResult = OnCaptureChangedMsg(bHandled);        
        //如果不设置bHandled，程序会转换为WM_BUTTON类消息
        bHandled = true;
        break;
    default:
        break;
    }
    return 0;
}

LRESULT WindowBase::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    switch (uMsg)
    {
        case WM_SIZE:
        {
            WindowSizeType sizeType = static_cast<WindowSizeType>(wParam);
            UiSize newWindowSize;
            newWindowSize.cx = (int)(short)LOWORD(lParam);
            newWindowSize.cy = (int)(short)HIWORD(lParam);
            lResult = OnSizeMsg(sizeType, newWindowSize, bHandled);
            break;
        }
        case WM_MOVE:
        {
            UiPoint ptTopLeft;
            ptTopLeft.x = (int)(short)LOWORD(lParam);   // horizontal position 
            ptTopLeft.y = (int)(short)HIWORD(lParam);   // vertical position 
            lResult = OnMoveMsg(ptTopLeft, bHandled);
            break;
        }
        case WM_PAINT:
        {
            lResult = OnPaintMsg(bHandled);
            break;
        }
        case WM_SETFOCUS:
        {
            lResult = OnSetFocusMsg(bHandled);
            break;
        }
        case WM_KILLFOCUS:
        {
            lResult = OnKillFocusMsg(bHandled);
            break;
        }
        case WM_IME_STARTCOMPOSITION:
        {
            lResult = OnImeStartCompositionMsg(bHandled);
            break;
        }
        case WM_IME_ENDCOMPOSITION:
        {
            lResult = OnImeEndCompositionMsg(bHandled);
            break;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT) {
                //只处理设置客户区的光标
                lResult = OnSetCursorMsg(bHandled);
            }            
            break;
        }
        case WM_CONTEXTMENU:
        {
            UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if ((pt.x != -1) && (pt.y != -1)) {
                ScreenToClient(pt);
            }
            lResult = OnContextMenuMsg(pt, bHandled);
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnKeyDownMsg(vkCode, modifierKey, bHandled);
            break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnKeyUpMsg(vkCode, modifierKey, bHandled);
            break;
        }
        case WM_CHAR:
        {
            VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnCharMsg(vkCode, modifierKey, bHandled);
            break;
        }
        case WM_HOTKEY:
        {
            int32_t hotkeyId = (int32_t)wParam;
            VirtualKeyCode vkCode = static_cast<VirtualKeyCode>((int32_t)(int16_t)HIWORD(lParam));
            uint32_t modifierKey = (uint32_t)(int16_t)LOWORD(lParam);
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnHotKeyMsg(hotkeyId, vkCode, modifierKey, bHandled);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseWheelMsg(wheelDelta, pt, modifierKey, bHandled);
            break;
        }        
        case WM_MOUSEMOVE:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseMoveMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_MOUSEHOVER:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseHoverMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_MOUSELEAVE:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseLeaveMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseLButtonDownMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_LBUTTONUP:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseLButtonUpMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_LBUTTONDBLCLK:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseLButtonDbClickMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseRButtonDownMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_RBUTTONUP:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseRButtonUpMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_RBUTTONDBLCLK:
        {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            uint32_t modifierKey = 0;
            GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = OnMouseRButtonDbClickMsg(pt, modifierKey, bHandled);
            break;
        }
        case WM_CAPTURECHANGED:
        {
            lResult = OnCaptureChangedMsg(bHandled);
            break;
        }
        case WM_CLOSE:
        {
            lResult = OnWindowCloseMsg((uint32_t)wParam, bHandled);
            break;
        }
        default:
            break;
    }//end of switch
    return lResult;
}

LRESULT WindowBase::OnSizeMsg(WindowSizeType sizeType, const UiSize& /*newWindowSize*/, bool& bHandled)
{
    bHandled = false;
    UiSize szRoundCorner = GetRoundCorner();
    bool isIconic = IsWindowMinimized();
    if (!isIconic && (sizeType != WindowSizeType::kSIZE_MAXIMIZED) && (szRoundCorner.cx > 0 && szRoundCorner.cy > 0)) {
        //最大化、最小化时，均不设置圆角RGN，只有普通状态下设置
        UiRect rcWnd;
        GetWindowRect(rcWnd);
        rcWnd.Offset(-rcWnd.left, -rcWnd.top);
        rcWnd.right++;
        rcWnd.bottom++;
        SetWindowRoundRectRgn(rcWnd, szRoundCorner, true);
    }
    else if (!isIconic) {
        //不需要设置RGN的时候，清除原RGN设置，避免最大化以后显示不正确
        ClearWindowRgn(true);
    }
    return 0;
}

LRESULT WindowBase::OnMoveMsg(const UiPoint& /*ptTopLeft*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnPaintMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnSetFocusMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnKillFocusMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnImeStartCompositionMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnImeEndCompositionMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnSetCursorMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnContextMenuMsg(const UiPoint& /*pt*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

bool WindowBase::GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const
{
    bool bRet = true;
    modifierKey = ModifierKey::kNone;
    switch (message) {
    case WM_CHAR:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;

    case WM_MOUSEWHEEL:
        {
            WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
            if (fwKeys & MK_CONTROL) {
                modifierKey |= ModifierKey::kControl;
            }
            if (fwKeys & MK_SHIFT) {
                modifierKey |= ModifierKey::kShift;
            }
        }
        break;
    case WM_MOUSEHOVER:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        if (wParam & MK_CONTROL) {
            modifierKey |= ModifierKey::kControl;
        }
        if (wParam & MK_SHIFT) {
            modifierKey |= ModifierKey::kShift;
        }
        break;
    default:
        bRet = false;
        break;
    }
    ASSERT(bRet);
    return bRet;
}

LRESULT WindowBase::OnKeyDownMsg(VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnKeyUpMsg(VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnCharMsg(VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnHotKeyMsg(int32_t /*hotkeyId*/, VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseWheelMsg(int32_t /*wheelDelta*/, const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseMoveMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseHoverMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseLeaveMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseLButtonDownMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseLButtonUpMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseLButtonDbClickMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseRButtonDownMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseRButtonUpMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnMouseRButtonDbClickMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnCaptureChangedMsg(bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT WindowBase::OnWindowCloseMsg(uint32_t /*wParam*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

} // namespace ui