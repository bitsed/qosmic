-- A simple loop that rotates all xforms around the origin

g = frame:get_genome()

while not stopped() do
	for i = 0, g:num_xforms() - 1 do
		local xf = g:get_xform(i)
		if xf:symmetry() == 0.0 then
			xf:rotate(-4.0, 0, 0);
		end
	end
	frame:render()
end


