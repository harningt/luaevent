package="luaevent"
version="scm-1"
source = {
   url = "git://github.com/harningt/luaevent.git",
}
description = {
   summary = "libevent binding for Lua",
   detailed = [[
       This is a binding of libevent to Lua
   ]],
   homepage = "https://github.com/harningt/luaevent",
   license = "MIT"
}
dependencies = {
   "lua >= 5.1, <= 5.4"
}
external_dependencies = {
   EVENT = {
      header = "event.h",
      library = "event",
   }
}
build = {
   type = "builtin",
   modules = {
      ["luaevent.core"] = {
         sources = { "src/buffer_event.c", "src/event_buffer.c", "src/event_callback.c", "src/utility.c", "src/luaevent.c" },
         libdirs = "$(EVENT_LIBDIR)",
         incdirs = { "include", "$(EVENT_INCDIR)" },
         libraries = "event",
      },
      ["luaevent"] = "lua/luaevent.lua",
   },
   copy_directories = { "doc", "test" },
}
