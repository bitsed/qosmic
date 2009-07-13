--
-- Perform a random walk on the nodes of each non-symmetry transform.
--
PI=3.14159265434
LENGTH_MULT=0.01

g = frame:get_genome()

while true do
	for i = 0, g:num_xforms() - 1 do
		local xf = g:get_xform(i)
		if xf:symmetry() == 0.0 then
			xf:rotate(-4.0, 0, 0);
			local x, y
			local a = math.random() * PI
			local l = math.random() * LENGTH_MULT
			dy = l*math.sin(a)
			dx = l*math.cos(a)
			x, y = xf:a()
			x, y = x+dx, y+dy
			xf:a(x, y)

			a = math.random() * PI
			l = math.random() * LENGTH_MULT
			dy = l*math.sin(a)
			dx = l*math.cos(a)
			x, y = xf:b()
			x, y = x+dx, y+dy
			xf:b(x, y)

			a = math.random() * PI
			l = math.random() * LENGTH_MULT
			dy = l*math.sin(a)
			dx = l*math.cos(a)
			x, y = xf:c()
			x, y = x+dx, y+dy
			xf:c(x, y)
		end
	end
	frame:render()
end

