#include "../message/message.h"
#include "../wnd/desktop.h"
#include "../wnd/reflow_queue.h"
#include "../wnd/redraw_queue.h"

#include "win32_api.h"
#include "win32_ime_input.h"
#include "win32_helper.h"


BEGIN_NAMESPACE(WndDesign)

bool size_move_entered = false;  // referenced in timer.cpp
ref_ptr<DesktopWndFrame> mouse_tracked_frame = nullptr;  // referenced in desktop.cpp

void CommitQueue() {
    static ReflowQueue& reflow_queue = GetReflowQueue();
    static RedrawQueue& redraw_queue = GetRedrawQueue();
    reflow_queue.Commit();
    redraw_queue.Commit();
}


BEGIN_NAMESPACE(Anonymous)


inline bool IsMouseMsg(UINT msg) { return WM_MOUSEFIRST <= msg && msg <= WM_MOUSELAST; }
inline bool IsKeyboardMsg(UINT msg) { return WM_KEYFIRST <= msg && msg <= WM_KEYLAST; }

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // Get the attached frame.
    DesktopWndFrame* frame = reinterpret_cast<DesktopWndFrame*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (frame == nullptr) { goto FrameIrrelevantMessages; }

    //// mouse message ////
    if (IsMouseMsg(msg)) {
        MouseMsg mouse_msg; Msg msg_type;
        mouse_msg.point = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        mouse_msg._key_state = GET_KEYSTATE_WPARAM(wParam);;
        mouse_msg.wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
        switch (msg) {
        case WM_MOUSEMOVE:
            msg_type = Msg::MouseMove;
            // Track mouse message for mouse leave.
            if (mouse_tracked_frame == nullptr) {
                TRACKMOUSEEVENT track_mouse_event;
                track_mouse_event.cbSize = sizeof(TRACKMOUSEEVENT);
                track_mouse_event.dwFlags = TME_LEAVE;
                track_mouse_event.hwndTrack = hWnd;
                track_mouse_event.dwHoverTime = HOVER_DEFAULT;
                TrackMouseEvent(&track_mouse_event);
                mouse_tracked_frame = frame;
                frame->ReceiveMessage(Msg::MouseEnter, nullmsg);
            }
            break;
        case WM_LBUTTONDOWN: msg_type = Msg::LeftDown; break;
        case WM_LBUTTONUP: msg_type = Msg::LeftUp; break;
        case WM_RBUTTONDOWN: msg_type = Msg::RightDown; break;
        case WM_RBUTTONUP: msg_type = Msg::RightUp; break;
        case WM_MBUTTONDOWN: msg_type = Msg::MiddleDown; break;
        case WM_MBUTTONUP: msg_type = Msg::MiddleUp; break;
        case WM_MOUSEWHEEL: msg_type = Msg::MouseWheel;
            // The point is relative to the desktop, convert it to the point on window.
            mouse_msg.point = mouse_msg.point + frame->GetRegionOffset();
            break;
        case WM_MOUSEHWHEEL: msg_type = Msg::MouseWheelHorizontal; 
            mouse_msg.point = mouse_msg.point + frame->GetRegionOffset();
            break;
        default: return DefWindowProc(hWnd, msg, wParam, lParam);
        }
        frame->ReceiveMessage(msg_type, mouse_msg);
        return 0;
    }

    //// keyboard message ////
    if (IsKeyboardMsg(msg)) {
        KeyMsg key_msg;
        key_msg.key = static_cast<Key>(wParam);
        key_msg._as_unsigned = static_cast<uint>(lParam);
        CharMsg char_msg;
        char_msg.ch = static_cast<wchar>(wParam);
        switch (msg) {
        case WM_KEYDOWN: frame->ReceiveMessage(Msg::KeyDown, key_msg); break;
        case WM_KEYUP: frame->ReceiveMessage(Msg::KeyUp, key_msg); break;
        case WM_CHAR: frame->ReceiveMessage(Msg::Char, char_msg); break;
        default: return DefWindowProc(hWnd, msg, wParam, lParam);
        }
        return 0; // never reached
    }

    //// ime message, region related message and other frame related messages ////
    {
        ImeInput& ime = ImeInput::Get();
        switch (msg) {
        case WM_IME_SETCONTEXT:
            ime.CreateImeWindow(hWnd);
            ime.CleanupComposition(hWnd);
            ime.SetImeWindowStyle(hWnd, msg, wParam, lParam);
            break;
        case WM_INPUTLANGCHANGE:
            ime.UpdateInputLanguage();
            break;
        case WM_IME_STARTCOMPOSITION:
            ime.CreateImeWindow(hWnd);
            ime.ResetComposition();
            frame->ReceiveMessage(Msg::ImeCompositionBegin, nullmsg);
            break;
        case WM_IME_COMPOSITION:
            ime.UpdateImeWindow(hWnd);
            if (ime.UpdateComposition(hWnd, lParam)) {
                ImeCompositionMsg ime_composition_msg(ime.GetComposition());
                frame->ReceiveMessage(Msg::ImeComposition, ime_composition_msg);
            }
            ime.UpdateResult(hWnd, lParam);
            break;
        case WM_IME_ENDCOMPOSITION:
            {
                ImeCompositionMsg ime_composition_msg(ime.GetResult());
                frame->ReceiveMessage(Msg::ImeCompositionEnd, ime_composition_msg); 
            }
            ime.ResetComposition();
            ime.DestroyImeWindow(hWnd);
            break;

            //// region message //// 
        case WM_ENTERSIZEMOVE:
            size_move_entered = true;
            break;
        case WM_EXITSIZEMOVE:
            size_move_entered = false;
            break;
        case WM_GETMINMAXINFO: {
                if (frame == nullptr) { break; }
                MINMAXINFO* min_max_info = reinterpret_cast<MINMAXINFO*>(lParam);
                auto [min_size, max_size] = frame->CalculateMinMaxSize();
                min_max_info->ptMaxPosition = { 0,0 };
                min_max_info->ptMaxSize = { static_cast<LONG>(max_size.width), static_cast<LONG>(max_size.height) };
                min_max_info->ptMinTrackSize = { static_cast<LONG>(min_size.width), static_cast<LONG>(min_size.height) };
                min_max_info->ptMaxTrackSize = { static_cast<LONG>(max_size.width), static_cast<LONG>(max_size.height) };
            }break;
        case WM_WINDOWPOSCHANGING:
            break;
        case WM_WINDOWPOSCHANGED: {
                WINDOWPOS* position = reinterpret_cast<WINDOWPOS*>(lParam);
                if ((position->flags & SWP_NOSIZE) && (position->flags & SWP_NOMOVE)) { break; }  // Filter out other messages.
                Rect rect(Point(position->x, position->y), Size(static_cast<uint>(position->cx), static_cast<uint>(position->cy)));
                frame->SetRegion(rect); CommitQueue();
            }break;
        case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                frame->Invalidate(RECT2Rect(ps.rcPaint));
                EndPaint(hWnd, &ps);
            }break;

        case WM_MOUSELEAVE: frame->OnMouseLeave(); break;
        case WM_CAPTURECHANGED:frame->LoseCapture(); break;
        case WM_KILLFOCUS: frame->LoseFocus(); break;

            // convert scroll message to mouse wheel message
        case WM_HSCROLL:
        case WM_VSCROLL: 
            {
                POINT cursor_position;
                GetCursorPos(&cursor_position);
                short key_state = 0;
                if (GetAsyncKeyState(VK_SHIFT)) { key_state |= MK_SHIFT; }
                if (GetAsyncKeyState(VK_CONTROL)) { key_state |= MK_CONTROL; }
                short wheel_delta = 0;
                switch (LOWORD(wParam)) {
                case SB_LINEUP: case SB_PAGEUP:wheel_delta = WHEEL_DELTA; break;
                case SB_LINEDOWN:case SB_PAGEDOWN:wheel_delta = -WHEEL_DELTA; break;
                default: return 0;
                }
                return WndProc(
                    hWnd, msg == WM_HSCROLL ? WM_MOUSEHWHEEL : WM_MOUSEWHEEL,
                    (wheel_delta << 16) | key_state, ((short)cursor_position.y << 16) | (short)cursor_position.x
                );
            }
        default: goto FrameIrrelevantMessages;
        }
        return 0;
    }

    //// frame irrelevant message ////
FrameIrrelevantMessages:
    switch (msg) {
    case WM_CREATE: break;
    case WM_DESTROY: if (frame != nullptr) { frame->OnDestroy(); }  break;

        // Intercept all non-client messages.
    case WM_NCCALCSIZE: break;  // Process the message to set client region the same as the window region.
    case WM_NCACTIVATE: return TRUE;  // Do not draw the nonclient area.
    case WM_NCHITTEST: return HTCLIENT;  // There's no non-client region.
    //case WM_NCCREATE: return TRUE; // Leave the message alone, or title won't get displayed.
    //case WM_NCDESTROY: break; // It doesn't matter.
    case WM_NCPAINT: break;

    case WM_ERASEBKGND: break;  // Intercept WM_ERASEBKGND from DefWindowProc(), else there may be problem using driect2d.
   
    default: return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;  // The message is handled.
}


static const wchar_t wnd_class_name[] = L"WndDesignFrame";
HINSTANCE hInstance = NULL;

inline void RegisterWndClass() {
    static bool has_wnd_class_registered = false;
    if (!has_wnd_class_registered) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = (hInstance = GetModuleHandle(NULL));
        wcex.lpszClassName = wnd_class_name;
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        ATOM res = RegisterClassExW(&wcex);
        if (res == 0) { throw std::runtime_error("register class error"); }
        has_wnd_class_registered = true;
    }
}

END_NAMESPACE(Anonymous)

BEGIN_NAMESPACE(Win32)


HANDLE CreateWnd(Rect region, const wstring& title, CompositeEffect composite_effect, std::function<void(HANDLE)> callback) {
    RegisterWndClass(); 
    HWND hWnd = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP, wnd_class_name, title.c_str(),
                                WS_POPUP | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_HSCROLL | WS_VSCROLL,
                                region.point.x, region.point.y, region.size.width, region.size.height,
                                NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) { throw std::runtime_error("create window error"); }
    if (callback) { callback(hWnd); }
    SetWndCompositeEffect(hWnd, composite_effect);
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    return hWnd;
}

void DestroyWnd(HANDLE hWnd) {
    DestroyWindow((HWND)hWnd);
}

void SetWndUserData(HANDLE hWnd, void* data) {
    SetWindowLongPtrW((HWND)hWnd, GWLP_USERDATA, (LONG_PTR)data);
}

void MoveWnd(HANDLE hWnd, Rect region) {
    MoveWindow((HWND)hWnd, region.point.x, region.point.y, region.size.width, region.size.height, false);
}

void SetWndTitle(HANDLE hWnd, const wstring& title) {
    SetWindowTextW((HWND)hWnd, title.c_str());
}

void SetWndCompositeEffect(HANDLE hWnd, CompositeEffect composite_effect) {
    // opacity and mouse-penerate
    LONG old_style = GetWindowLong((HWND)hWnd, GWL_EXSTYLE);
    if (!(old_style & WS_EX_LAYERED)) {
        if (composite_effect._opacity == 0xFF && composite_effect._mouse_penetrate == false) {
            goto SetZIndex;
        }
        old_style |= WS_EX_LAYERED;
    }
    if (composite_effect._mouse_penetrate == true) {
        old_style |= WS_EX_TRANSPARENT;
    } else {
        old_style &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLong((HWND)hWnd, GWL_EXSTYLE, old_style);
    SetLayeredWindowAttributes((HWND)hWnd, 0, composite_effect._opacity, LWA_ALPHA);

SetZIndex:
    // z-index
    if (composite_effect.IsTopmost()) { SetWindowPos((HWND)hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); }
    if (composite_effect.IsBottom()) { SetWindowPos((HWND)hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); }
}

void SetCapture(HANDLE hWnd) {
    ::SetCapture((HWND)hWnd);
}

void ReleaseCapture() {
    ::ReleaseCapture();
}

void SetFocus(HANDLE hWnd) {
    ::SetFocus((HWND)hWnd);
}

void ReleaseFocus() {
    ::SetFocus(NULL);
}

int MessageLoop() {
    MSG msg;
    while (true) {
        GetMessageW(&msg, nullptr, 0, 0);
        do {
            if (msg.message == WM_QUIT) {
                return (int)msg.wParam;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        } while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE));

        CommitQueue();
    }
    assert(false); return 0;
}

void ExitMessageLoop() {
    PostQuitMessage(0);
}


END_NAMESPACE(Win32)

END_NAMESPACE(WndDesign)