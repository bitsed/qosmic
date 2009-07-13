-- Rotate each transform around the origin and render to
-- png files.  The png files can be combined into a movie.
g = frame:get_genome()

n=10

while true do
	local name = "/tmp/flam-img0" .. n .. ".png"
	
	for i = 0, g:num_xforms() - 1 do
		local xf = g:get_xform(i)
		if xf:symmetry() == 0.0 then
			xf:rotate(-4.0, 0, 0);
		end
	end
 	print("rendering " .. name)
	frame:render(0, name)
	n = n+1
end

