-- This script simply rotates the genome color palette.

gid  = 1 -- rotate the palette for this genome
step = 4 -- step over this many palette entries each loop

gen = frame:get_genome(gid)
while true do
	r, g, b = gen:palette( (1 + step) % 255 )

	for n = 2, 256 do
		local r, g, b = gen:palette( (n + step) % 255 )
		gen:palette(n - 1, r, g, b)
	end
	gen:palette(256, r, g, b)

	frame:render(gid)
end
