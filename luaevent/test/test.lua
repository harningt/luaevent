-- Tests Copas with a simple Echo server
--
-- Run the test file and the connect to the server by telnet on the used port
-- to stop the test just send the command "quit"

require"luaevent"
require"socket"
local function echoHandler(skt)
	while true do
		local data,ret = luaevent.receive(skt, 10)
		if data == "quit" or ret == 'closed' then
			break
		end
		--collectgarbage()
		luaevent.send(skt, data)
	end
end

local server = assert(socket.bind("localhost", 20000))
server:settimeout(0)

luaevent.addserver(server, echoHandler)
luaevent.loop()