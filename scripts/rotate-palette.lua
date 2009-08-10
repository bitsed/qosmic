-- This script simply rotates the genome color palette.

gid   = 0 -- rotate the palette for this genome
step  = 4 -- step over this many palette entries each loop
 
gen = frame:get_genome(gid)
while true do
	r, g, b = gen:palette(0)
	
	for n = 1, 255 do
		local r, g, b = gen:palette( (n + step) % 255 )
		gen:palette(n - 1, r, g, b)
	end
	gen:palette(255, r, g, b)

	frame:render(gid)
end