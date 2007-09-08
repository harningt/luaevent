require("luaevent.core")
local buffer = luaevent.core.buffer

require("lunit")

lunit.import("all")

bufferTests = TestCase("bufferTests")

function bufferTests:setup()
	self.buffer = buffer.new()
	self.buffer2 = buffer.new()
end

function bufferTests:teardown()
	self.buffer:close()
	self.buffer2:close()
end

local function testDataEqual(expected, actual, msg)
	msg = msg or ''
	assert_equal(expected, actual:get_data(), "Buffer not the same: " .. msg)
	assert_equal(#expected, actual:length(), "Buffer length not the same: " .. msg)
	assert_equal(expected, tostring(actual), "Buffer (from tostring) not the same: " .. msg)
	assert_equal(#expected, #actual, "Buffer length (from #) not zero: " .. msg)
end

function bufferTests:test_empty()
	testDataEqual("", self.buffer, "Buffer not empty")
	testDataEqual("", self.buffer2, "Buffer2 not empty")
end

function bufferTests:test_addSimpleString()
	self.buffer:add("Hello")
	testDataEqual("Hello", self.buffer)
	self.buffer:add("Hello")
	testDataEqual("HelloHello", self.buffer)
end

function bufferTests:test_addMultipleStrings()
	self.buffer:add("Hello", "Hello")
	testDataEqual("HelloHello", self.buffer)
end

function bufferTests:test_addDigits()
	self.buffer:add(1,2,3,4)
	testDataEqual("1234", self.buffer)
	self.buffer:add(100)
	testDataEqual("1234100", self.buffer)
	self.buffer:add(1.1)
	testDataEqual("12341001.1", self.buffer)
end

function bufferTests:test_addBuffer()
	self.buffer:add(self.buffer2)
	testDataEqual("", self.buffer)
	testDataEqual("", self.buffer2)
	self.buffer2:add("Hello")
	testDataEqual("Hello", self.buffer2)
	self.buffer:add(self.buffer2)
	testDataEqual("Hello", self.buffer)
	testDataEqual("", self.buffer2)
	assert_error("Cannot self-add buffers", function()
		self.buffer:add(self.buffer)
	end)
	assert_error("Cannot self-add buffers", function()
		self.buffer2:add(self.buffer2)
	end)
	testDataEqual("Hello", self.buffer, "Failures should not affect data content")
	testDataEqual("", self.buffer2, "Failures should not affect data content")
end

function bufferTests:test_addBadValues_fail()
	assert_error("Should not be able to add no values", function()
		self.buffer:add()
	end)
	assert_error("Should not be able to add boolean true", function()
		self.buffer:add(true)
	end)
	assert_error("Should not be able to add boolean false", function()
		self.buffer:add(false)
	end)
	assert_error("Should not be able to add functions", function()
		self.buffer:add(function() end)
	end)
	assert_error("Should not be able to add threads", function()
		self.buffer:add(coroutine.create(function() end))
	end)
	assert_error("Should not be able to add non-buffer userdata", function()
		self.buffer:add(newproxy())
	end)
	assert_error("Should not be able to add nil values", function()
		self.buffer:add(nil)
	end)
	assert_error("Multiples including valid values should not affect failure results", function()
		self.buffer:add("Hello", 1, bufferb, true, false, function() end, newproxy(), nil)
	end)
	testDataEqual("", self.buffer, "Buffer not empty after failing additions")
end

function bufferTests:test_drain()
	self.buffer:add("123456789")
	testDataEqual("123456789", self.buffer)
	assert_error("Cannot call drain w/ no args", function()
		self.buffer:drain()
	end)
	self.buffer:drain(1)
	testDataEqual("23456789", self.buffer)
	self.buffer:drain(4)
	testDataEqual("6789", self.buffer)
	assert_pass("Should be able to apply draining beyond actual buffer length", function()
		self.buffer:drain(5)
	end)
	testDataEqual("", self.buffer)
end

lunit.run()