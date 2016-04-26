--
-- Electric sheep looping animation script.
-- Rotate each non-symmetry transform around the origin and render a preview.
--

GENOME_IDX = 1  -- animate the genome at this index

g = frame:get_genome(GENOME_IDX)

while true do
	for i = 1, g:num_xforms() do
		local xf = g:get_xform(i)
		if xf:animate() > 0.0 then
			xf:rotate(-4.0, 0, 0);
		end
	end
	frame:render(GENOME_IDX)
end
