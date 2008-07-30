-- This script scans the directory given by path for flam3 xml files without
-- a cooresponding png image file, and then generates one.  It requires
-- luafilesystem (luaforge.net).

require"lfs"
path="/home/user/flam3 images"
for file in lfs.dir(path) do
	if string.match(file,  "\.flam3$") then
		local f = path..'/'..file
		local attr = lfs.attributes(f)
		
		if attr.mode == "file" then
			i = string.gsub(f, "\.flam3$", ".png")
			local imgattr = lfs.attributes(i)
			if (imgattr == nil) then
				print ('rendering ' .. i)
				frame:load(f)
				frame:render(0, i)
			end
		
		end
		
	end
end
