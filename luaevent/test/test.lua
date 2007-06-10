-- Tests Copas with a simple Echo server
--
-- Run the test file and the connect to the server by telnet on the used port
-- to stop the test just send the command "quit"

require"luaevent"
require"socket"
local function echoHandler(skt)
  while true do
  print(skt)
    local data,ret = luaevent.receive(skt, 10)
    print("GOT: ", data, ret)
    if data == "quit" or ret == 'closed' then
      break
    end
    luaevent.send(skt, data)
  end
  print("DONE")
end
local function setupHook(thread)
	if not thread then debug.sethook(function(event) print("TRACE >: ", debug.getinfo(2, 'n').name) end, 'c')
	else debug.sethook(thread, function(event) print("TRACE ", thread,">: ", debug.getinfo(2, 'n').name) end, 'c') end
end
local server = assert(socket.bind("localhost", 20000))
server:settimeout(0)
setupHook()
local coro = coroutine.create
coroutine.create = function(...)
	local ret = coro(...)
	setupHook(ret)
	return ret
end
luaevent.addserver(server, echoHandler)
luaevent.loop()
