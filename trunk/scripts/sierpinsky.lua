-- this script generates a sierpinski gasket

g = frame:get_genome();
g:clear_xforms();

g:width(640);
g:height(480);
g:scale(240);
g:center(1.80, -0.20);

xf = g:add_xform();
xf:scale(0.5);
xf = g:add_xform();
xf:translate(1.28, 0);
xf:scale(0.5)
xf = g:add_xform();
xf:translate(0.64, -0.8);
xf:scale(0.5);

nxforms = g:num_xforms();
for i = 0, nxforms - 1 do
  local xf = g:get_xform(i);
  xf:density(1 / nxforms);
  xf:color(i / (nxforms - 1));
  xf:var(LINEAR, 1.0)
end

frame:render()
