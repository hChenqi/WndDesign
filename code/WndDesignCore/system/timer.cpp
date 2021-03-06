#include "timer.h"

#include <unordered_map>
#include <Windows.h>


BEGIN_NAMESPACE(WndDesign)

extern bool size_move_entered;  // defined in win32_api.cpp
extern void CommitQueue();

BEGIN_NAMESPACE(Anonymous)

std::unordered_map<HANDLE, Timer&> timer_sync_map;

void TimerCallbackSync(HWND Arg1, UINT Arg2, UINT_PTR Arg3, DWORD Arg4) {
    auto it = timer_sync_map.find(reinterpret_cast<HANDLE>(Arg3));
    if (it == timer_sync_map.end()) { return; }
    it->second.callback();
    if (size_move_entered) { CommitQueue(); }
}

HANDLE SetTimerSync(uint period, Timer& timer_object) {
    HANDLE timer = reinterpret_cast<HANDLE>(SetTimer(NULL, NULL, period, TimerCallbackSync));
    timer_sync_map.emplace(timer, timer_object);
    return timer;
}

void ResetTimerSync(HANDLE timer, uint period) {
    assert(timer_sync_map.find(timer) != timer_sync_map.end());
    SetTimer(NULL, reinterpret_cast<UINT_PTR>(timer), period, TimerCallbackSync);
}

void KillTimerSync(HANDLE timer) {
    auto it = timer_sync_map.find(timer);
    assert(it != timer_sync_map.end());
    KillTimer(NULL, reinterpret_cast<UINT_PTR>(timer));
    timer_sync_map.erase(it);
}

END_NAMESPACE(Anonymous)


void Timer::Set(uint period) {
    if (!IsSet()) {
        timer = SetTimerSync(period, *this);
    } else {
        ResetTimerSync(timer, period);
    }
}

void Timer::Stop() {
	if (!IsSet()) { return; }
    KillTimerSync(timer);
    timer = nullptr;
}


END_NAMESPACE(WndDesign)