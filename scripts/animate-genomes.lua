-- =============================================================
-- Linearly interpolate genomes 'A' through genome 'B' and save
-- into genome 'C'.
--
-- A is the starting genome index
-- B is the ending genome index (> A)
-- C is the interpolated genome index (> B)
--
-- LIN_STEPS is the number of linear interpolation steps
-- ROT_STEPS is the number of rotational steps (degrees)
-- ROT_LOOPS is the number of rotational loops per genome
--
A, B, C = 0, 1, 2
LIN_STEPS = 36
ROT_STEPS = 36
ROT_LOOPS = 1
-- =============================================================

function init_xf(ga)
		ga.x, ga.y = 0, 0
		ga.scale = 100
		ga.ax, ga.ay = 0, 0
		ga.bx, ga.cy = 1, 1
		ga.cx, ga.by = 0, 0
		ga.apx, ga.apy = 0, 0
		ga.bpx, ga.cpy = 1, 1
		ga.cpx, ga.bpy = 0, 0
		ga.color, ga.density = 0, 0
		ga.default = true
end
	
function interp_genomes_to(genomea, genomeb, genomec, t) 
	local ga, gb = {}, {}
	ga.nxforms = genomea:num_xforms()
	gb.nxforms = genomeb:num_xforms()
	
	for n = 0, (nxf - 1) do
		init_xf(ga)
		init_xf(gb)
		if n < ga.nxforms then
			local xf = genomea:get_xform(n)
			ga.ax, ga.ay = xf:a()
			ga.bx, ga.by = xf:b()
			ga.cx, ga.cy = xf:c()
-- 			ga.apx, ga.apy = xf:ap()
-- 			ga.bpx, ga.bpy = xf:bp()
-- 			ga.cpx, ga.cpy = xf:cp()
			ga.density = xf:density()
			ga.color = xf:color()
			ga.default = false;
		end

		if n < gb.nxforms then
			local xf = genomeb:get_xform(n)
			gb.ax, gb.ay = xf:a()
			gb.bx, gb.by = xf:b()
			gb.cx, gb.cy = xf:c()
-- 			gb.apx, gb.apy = xf:ap()
-- 			gb.bpx, gb.bpy = xf:bp()
-- 			gb.cpx, gb.cpy = xf:cp()
			gb.density = xf:density()
			gb.color = xf:color()
			gb.default = false;
		end

		local xf = genomec:get_xform(n)
		local x,y
		x = ga.ax + t * (gb.ax - ga.ax)
		y = ga.ay + t * (gb.ay - ga.ay)
		xf:a(x, y)
		x = ga.bx + t * (gb.bx - ga.bx)
		y = ga.by + t * (gb.by - ga.by)
		xf:b(x, y)
		x = ga.cx + t * (gb.cx - ga.cx)
		y = ga.cy + t * (gb.cy - ga.cy)
		xf:c(x, y)
-- 		x = ga.apx + t * (gb.apx - ga.apx)
-- 		y = ga.apy + t * (gb.apy - ga.apy)
-- 		xf:ap(x, y)
-- 		x = ga.bpx + t * (gb.bpx - ga.bpx)
-- 		y = ga.bpy + t * (gb.bpy - ga.bpy)
-- 		xf:bp(x, y)
-- 		x = ga.cpx + t * (gb.cpx - ga.cpx)
-- 		y = ga.cpy + t * (gb.cpy - ga.cpy)
-- 		xf:cp(x, y)
		
		for j = 0, NUM_VARS do
			local va, vb = 0, 0
			if ga.default then
				va = 0
			else
			 	va = genomea:get_xform(n):var(j)
			end
			if gb.default then
				vb = 0
			else
				vb = genomeb:get_xform(n):var(j)
			end
			xf:var(j, va + t * (vb - va))
		end

		xf:color(ga.color + t *(gb.color - ga.color))
		xf:density(ga.density + t *(gb.density - ga.density))
		
	end
	
	for k = 0, 255 do
		local ar,ag,ab	 = genomea:palette(k)
		local br,bg,bb	 = genomeb:palette(k)
		local r = ar + t * (br - ar)
		local g = ag + t * (bg - ag)
		local b = ab + t * (bb - ab)
		genomec:palette(k, r, g, b)
	end
	
	ga.x, ga.y = genomea:center()
	gb.x, gb.y = genomeb:center()

	local x = ga.x + t*(gb.x - ga.x)
	local y = ga.y + t*(gb.y - ga.y)
	genomec:center(x, y)
	
	ga.scale = genomea:scale()
	gb.scale = genomeb:scale()
	genomec:scale(ga.scale + t*(gb.scale -  ga.scale))
end


--- BEGIN ---
num_genomes = frame:num_genomes()
if A == nil then A = 0 end
if B == nil then B = num_genomes - 1 end
if C == nil then C = num_genomes - 0 end

for n = A, B - 1 do
	genomeA = frame:get_genome(n)
	genomeB = frame:get_genome(n+1)
	frame:copy_genome(n, C)
	genomeC = frame:get_genome(C)
	
	local deg = -360. / ROT_STEPS
	for k = 0, ROT_STEPS * ROT_LOOPS do
		for i = 0, genomeC:num_xforms() - 1 do
			local xf = genomeC:get_xform(i)
			if xf:symmetry() == 0.0 then
				xf:rotate(deg, 0, 0);
			end
		end
		frame:render(C)
	end
	
	a_nxf = genomeA:num_xforms()
	b_nxf = genomeB:num_xforms()
	nxf = a_nxf
	if b_nxf > a_nxf then
		nxf = b_nxf
	end
	if nxf > a_nxf then
		genomeC:add_xform(nxf - a_nxf)
	end
	
	for i = 0, LIN_STEPS do
		local t  = i / LIN_STEPS
		interp_genomes_to(genomeA, genomeB, genomeC, t)
		frame:render(C)
	end
	frame:del_genome(C)
end
