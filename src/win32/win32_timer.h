struct Win32_Timer {
    LARGE_INTEGER counts_per_second;
    LARGE_INTEGER begin_frame;
    b32 sleep_is_granular;
};

internal b32 win32_timer_init(Win32_Timer* timer);
internal void win32_timer_begin_frame(Win32_Timer* timer);
internal void win32_timer_end_frame(Win32_Timer* timer, f64 milliseconds_per_frame);
