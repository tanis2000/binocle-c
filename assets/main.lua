--
-- Author: Valerio Santinelli <santinelli@gmail.com>
-- Date: 17/02/17
--

--package.path="../Resources/?.lua;./?.lua"
print("Checking is LUAJIT is available")
if jit ~= nil then
        print("LUAJIT is available")
        print("LUAJIT version " .. jit.version)
        print("LUAJIT engine status is " .. tostring(jit.status()))
end

local testModule = require("test")
testModule.foo()

socket = require("socket")
print(socket._VERSION)
-- this should have an if debug in front as it's 10x slower when mobdebug is on
require("mobdebug").start()

testString = "LuaBridge works!"
number = 42

log.log("Starting main.lua")

function tellme(offset, story)
        local n,v
        for n,v in pairs(story) do
                if n ~= "loaded" and n ~= "_G" then
                io.write (offset .. n .. " " )
                print (v)
                if type(v) == "table" then
                        tellme(offset .. "--> ",v)
                end
                end
        end
end

tellme("",_G)

function update()
    log.log("update loop")
end

function draw()
end