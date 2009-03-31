-- This script will normalize the xform densities to one.
-- Running the script will have no effect on the flam3 image,
-- since the densities are relative probabilities.
sumq = 0
g = frame:get_genome()
for it = 0, g:num_xforms() - 1 do
    local xf = g:get_xform(it)
    sumq = sumq + xf:density()^2
end

sumq = math.sqrt(sumq)

for it = 0, g:num_xforms() - 1 do
    local xf = g:get_xform(it)
    xf:density(xf:density() / sumq)
end
