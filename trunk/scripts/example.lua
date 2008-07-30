-- Here are some examples of how to use lua scripts with qosmic.  Be sure to
-- also examine the functions.lua script.

-- load and save functions
-- frame:load('sparkler.flam3')
-- frame:save('sparkler.flam3')

g = frame:get_genome() ;  -- set g to refer to the first genome
r = frame:get_genome(0) ; -- both g and r point to the same genome
n = frame:get_genome(1) ; -- the second genome, create a new genome if necessary

xf = g:get_xform(0) ;         -- set xf to point at the first xform in r
density = xf:density()        -- get the density
xf:density(1.0 + density) ;   -- add one to the xform density
xf:var(LINEAR, 2.0) ;         -- set the linear variation to 2.0
(a, b, c) = xf:a(), xf:b(), xf:c() ; -- coordinates
(x,y) = xf:pos(-1.0,1.2) ; -- position the center of the triangle at (-1.0, 1.2)

xf_2 = g:add_xform() ;      -- set xf_2 to point at a new xform
xf_2:translate(1.0, 0.0) ;  -- move the xform
xf_2:rotate(60) ;           -- rotate (in degrees) the xform

ngenomes = frame:num_genomes() ;

frame:render() ; -- render the first genome, and update the preview image
frame:render(1, "image.png") ; -- render the second genome, and save to a file
frame:render(n) ;              -- render the second genome

frame:copy_genome(0, 1) ; -- copy genome 0 to genome 1
frame:del_genome(1) ;     -- delete genome 1
