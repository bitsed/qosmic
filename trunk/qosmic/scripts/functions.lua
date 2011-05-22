function dump_xform(xf)
	print(xf)
	print('index=',xf:index())
	print('pos=',xf:pos())
	print('color=',xf:color())
	print('density=',xf:density())
	print('color_speed=',xf:color_speed())
	print('opacity=',xf:opacity())
	print('a=',xf:a())
	print('b=',xf:b())
	print('c=',xf:c())
	for _, v in ipairs(VARIATIONS) do
		val, vars = xf:var(_)
		if val ~= 0 then
			print(v .. '=', val)
			for _, v in pairs(vars) do
				print('    ' .. _ .. '=', v)
			end
		end
	end
end

function dump_genome(g)
	print(g)
	nxf = g:num_xforms()
	print('num_xforms=',nxf)
	print('size=',g:width(),g:height())
	print('time=',g:time())
	print('final_xform_index=',g:final_xform_index())
	print('center=',g:center())
	print('rotate=',g:rotate())
	print('zoom=',g:zoom())
	print('nbatches=',g:nbatches())
	print('estimator=',g:estimator())
	print('estimator_curve=',g:estimator_curve())
	print('estimator_minimum=',g:estimator_minimum())
	print('sample_density=',g:sample_density())
	print('scale=',g:pixels_per_unit())
	print('background=', g:background())
	print()
	if (nxf > 0) then
		for n = 1, nxf do
			dump_xform(g:get_xform(n))
			print()
		end
	end
end

function set_low_quality(g)
	g:spatial_filter_radius(0)
	g:sample_density(40)  -- quality
	g:ntemporal_samples(1)
	g:estimator(0)
	g:nbatches(1)  -- passes
end

function modify_xf_test(xf,d)
	for n=1,100 do
		xf:translate(d*0.05,0)
		xf:rotate(3.6)
		xf:scale(0.95)
		frame:render()
	end
end

function rotate_xf_test(xf)
	for n=1,360 do
		xf:rotate(1)
		frame:update() -- update the widgets' data
	end
end

function spread_colors(genome)
-- Spreads the color parameters evenly over all
-- xforms in genome
	local num_xf = genome:num_xforms()
	for i = 1, num_xf do
		genome:get_xform(i):color(i / num_xf);
	end
end

function spread_density(g)
-- Spreads the xform density parameters evenly over 1.0
	local num_xf = genome:num_xforms()
	for i = 1, num_xf do
		genome:get_xform(i):density(1.0 / num_xf);
	end
end

function bw_palette(g)
-- Creates a white to black gradient in the palette for genome g
	for i = 1,256 do
		v = (256-i)/256
		g:palette(i, v,v,v)
	end
end
