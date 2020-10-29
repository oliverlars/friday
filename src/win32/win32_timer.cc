
internal b32
win32_timer_init(Win32_Timer* timer){
    
    b32 result = 0;
    
    if(QueryPerformanceFrequency(&timer->counts_per_second)){
        result = 1;
    }
    
    timer->sleep_is_granular =  (timeBeginPeriod(1) == TIMERR_NOERROR);
    
    return result;
}

internal void
win32_timer_begin_frame(Win32_Timer* timer){
    
    QueryPerformanceFrequency(&timer->begin_frame);
}

internal void
win32_timer_end_frame(Win32_Timer* timer, f64 milliseconds_per_frame){
    LARGE_INTEGER end_frame;
    QueryPerformanceFrequency(&end_frame);
    
    f64 desired_seconds_per_frame = (milliseconds_per_frame / 1000.0);
    s64 elapsed_counts = end_frame.QuadPart - timer->begin_frame.QuadPart;
    s64 desired_counts = (s64)(desired_seconds_per_frame * timer->counts_per_second.QuadPart);
    s64 counts_to_wait = desired_counts - elapsed_counts;
    
    LARGE_INTEGER start_wait;
    LARGE_INTEGER end_wait;
    
    QueryPerformanceCounter(&start_wait);
    
    while(counts_to_wait > 0){
        if(timer->sleep_is_granular){
            DWORD milliseconds_to_sleep = (DWORD)(1000.0 * ((f64)(counts_to_wait) / timer->counts_per_second.QuadPart));
            if(milliseconds_to_sleep > 0){
                Sleep(milliseconds_to_sleep);
            }
        }
        
        QueryPerformanceCounter(&end_wait);
        counts_to_wait -= end_wait.QuadPart - start_wait.QuadPart;
        start_wait = end_wait;
    }
}

