local ffi = require("ffi")
ffi.cdef[[

void binocle_window_set_title(struct binocle_window *win, char *title);

]]
io.write("Starting up FFI test script\n");
local title = "Lua FFI"
local c_str = ffi.new("char[?]", #title)
ffi.copy(c_str, title)
ffi.C.binocle_window_set_title(window, c_str)
io.write("Terminating FFI test script\n");
