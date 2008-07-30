
PI=3.14159265434
LENGTH_MULT=0.005

g = frame:get_genome()

n=10

while true do
	local name = "/tmp/flam3-img0" .. n .. ".png"
	for i = 0, g:num_xforms() - 1 do
		local xf = g:get_xform(i)
		if xf:symmetry() == 0.0 then
			xf:rotate(-4.0, 0, 0);
			local x, y
			local a = irand() * 2 * PI
			local l = irand() * LENGTH_MULT
			dy = l*math.sin(a)
			dx = l*math.cos(a)
			x, y = xf:a()
			x, y = x+dx, y+dy
			xf:a(x, y)

			a = irand() * 2 * PI
			l = irand() * LENGTH_MULT
			dy = l*math.sin(a)
			dx = l*math.cos(a)
			x, y = xf:b()
			x, y = x+dx, y+dy
			xf:b(x, y)

			a = irand() * 2 * PI
			l = irand() * LENGTH_MULT
			dy = l*math.sin(a)
			dx = l*math.cos(a)
			x, y = xf:c()
			x, y = x+dx, y+dy
			xf:c(x, y)
		end
	end
	print("rendering " .. name)
	frame:render() --0, name)
	n = n+1
end
