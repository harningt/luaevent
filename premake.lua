project.name = "luaevent.core"
project.libdir = "lib"
project.bindir = "bin"

package = newpackage()
package.kind = "dll"
package.language = "c++"
package.targetprefix = ""
package.target = "core"

package.links = {
	"event"
}

package.includepaths = {
	"include",
}
if linux then
	package.buildoptions = { "-Wall" }
	package.config["Debug"].buildoptions = { "-O0" }
	package.linkoptions =  { "-Wall -L/usr/local/lib" }
	package.postbuildcommands = { "mkdir -p test/luaevent", "cp bin/* test/luaevent", "cp luaevent.lua test" }
else
	print([[Other environements currently untested, may need tweaking]])
end

package.files = {
	matchrecursive(
		"src/*.c",
		"include/*.h"
	)
}
