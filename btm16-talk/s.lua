sysnative = not not package.loadlib

local SHADETAB = {" ", "█"}
local FADETAB = { " ","░","▒","▓","█" }

local component = (not sysnative) and require("component")
local term = (not sysnative) and require("term")
local gpu = (not sysnative) and component.gpu

if not sysnative then
	term.clear()
end

local W,H = 160, 49
if not sysnative then
	W,H = gpu.getResolution()
	W = W - 1
	H = H - 1
end

local slide = ...
local all_args = {...}
slide = math.tointeger(slide)

local sdata = {}
local x,y

for y=1,H do
	sdata[y] = {}
	for x=1,W do
		sdata[y][x] = " "
	end
end

function waveform(x)
	--return -math.sin(x*math.pi*2.0/W*3.0)
	x = x*math.pi*2.0/W*2.0
	return -math.sin(x + math.pi*math.sin(x/2.0)*1.5/2.0)
end

function slide_1()
	local x, y

	for x=1,W do
		y = waveform(x)
		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = SHADETAB[2]
	end
end

function slide_2()
	local x, y

	for x=1,W do
		local v = waveform(x)
		y = math.tointeger(math.floor((v+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[3]

		if v >= 0.0 then y = 1.0 else y = -1.0 end
		y = y * 0.5
		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = SHADETAB[2]
	end
end

function slide_3()
	local x, y
	local q = 0.0
	local s = tonumber(all_args[2]) or 0.1

	for x=1,W do
		local v = waveform(x)
		y = math.tointeger(math.floor((v+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[3]

		if v >= q then
			y = 1.0
			q = q + (1.0 - q)*s
		else
			y = -1.0
			q = q + (-1.0 - q)*s
		end
		y = y * 0.5

		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[2]
		y = math.tointeger(math.floor((q+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = SHADETAB[2]

	end
end

function slide_4()
	local x, y
	local q = 0.0
	local s = 1.0/255.0
	local Ri = 7.0/255.0
	local Rd = 20.0/255.0
	local last_targ = true

	for x=1,W do
		local v = waveform(x)
		y = math.tointeger(math.floor((v+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[3]

		local next_targ = (v >= q)
		if v >= q then
			y = 1.0
		else
			y = -1.0
		end
		q = q + (y - q)*s

		if last_targ == next_targ then
			s = s + (1.0 - s)*Ri
		else
			s = s + (0.0 - s)*Rd
		end

		y = y * 0.5

		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[2]
		y = math.tointeger(math.floor((q+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = SHADETAB[2]
		last_targ = next_targ
	end
end

function slide_5()
	local x, y
	local q = 0.0
	local s = 1.0/255.0
	local Ri = 7.0/255.0
	local Rd = 20.0/255.0
	local last_targ = true
	local l2_targ = true
	local lq = 0.0

	for x=1,W do
		local v = waveform(x)
		y = math.tointeger(math.floor((v+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[3]

		local next_targ = (v >= q)
		if v >= q then
			y = 1.0
		else
			y = -1.0
		end
		q = q + (y - q)*s

		if last_targ == next_targ then
			s = s + (1.0 - s)*Ri
		else
			s = s + (0.0 - s)*Rd
		end

		y = y * 0.5

		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[2]

		if last_targ == next_targ then
			y = q
		else
			y = (lq+q)/2.0
		end
		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = SHADETAB[2]

		lq = q
		last_targ = next_targ
	end
end

function slide_6()
	local x, y
	local q = 0.0
	local s = 1.0/255.0
	local Ri = 7.0/255.0
	local Rd = 20.0/255.0
	local last_targ = true
	local l2_targ = true
	local lq = 0.0
	local mq = 0.0
	local mS = tonumber(all_args[2]) or 0.5

	for x=1,W do
		local v = waveform(x)
		y = math.tointeger(math.floor((v+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[3]

		local next_targ = (v >= q)
		if v >= q then
			y = 1.0
		else
			y = -1.0
		end
		q = q + (y - q)*s

		if last_targ == next_targ then
			s = s + (1.0 - s)*Ri
		else
			s = s + (0.0 - s)*Rd
		end

		y = y * 0.5

		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = FADETAB[2]

		if last_targ == next_targ then
			y = q
		else
			y = (lq+q)/2.0
		end

		mq = mq + (y - mq) * mS
		y = mq
		y = math.tointeger(math.floor((y+1.0)*(H-1)/2.0+1.5))
		sdata[y][x] = SHADETAB[2]

		lq = q
		last_targ = next_targ
	end
end

if slide == 1 then
	slide_1()

elseif slide == 2 then
	slide_2()

elseif slide == 3 then
	slide_3()

elseif slide == 4 then
	slide_4()

elseif slide == 5 then
	slide_5()

elseif slide == 6 then
	slide_6()

else
	error("not a slide!")

end

for y=1,H do
	local s = ""

	for x=1,W do
		s = s .. sdata[y][x]
	end

	print(s)
end

