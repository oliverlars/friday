
internal void
xspacer(f32 space = 10.0f);

internal void
yspacer(f32 space = 10.0f);

internal void
ui_window(v4f rect, char* fmt, ...);

internal void
ui_popup(v4f rect, char* fmt, ...);

internal void
ui_container(char* fmt, ...);

internal void
ui_panel_header(Panel* panel, char* fmt, ...);

internal void
label(char* fmt, ...);

internal b32
arrow_dropdown(char* fmt, ...);

internal b32
arrow_dropdown2(char* fmt, ...);

internal b32
arrow_dropdown3(char* fmt, ...);

internal b32
button(char* fmt, ...);

internal b32
check(char* fmt, ...);

internal b32
check(b32* checked, char* fmt, ...);

internal b32
button_fixed(char* fmt, ...);

internal void
fslider(f32 min, f32 max, f32* value, char* fmt, ...);

internal b32
dropdown(char* fmt, ...);

internal void
text_box(String8* string);
