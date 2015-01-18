oldpad = Controls.read()
cia_table = System.listCIA()
not_extracted = true
function CropPrint(x, y, text, color, screen)
	if string.len(text) > 25 then
		Screen.debugPrint(x, y, string.sub(text,1,25) .. "...", color, screen)
	else
		Screen.debugPrint(x, y, text, color, screen)
	end
end
function listDirectory(dir)
	tab = System.listDirectory(dir)
	ret_tab = {}
	for i, file in pairs(tab) do
		if (file.directory) or (string.sub(file.name,-4) == ".cia") or (string.sub(file.name,-4) == ".CIA") then
			table.insert(ret_tab, file)
		end
	end
	return ret_tab
end
files_table = listDirectory("/")
master_index = 0
p = 1
mode = "CIA"
selected_item = Color.new(255,0,0)
menu_color = Color.new(255,255,255)
selected_color = Color.new(255,255,0)
function GetCategory(c)
	if c==0 then
		return "Application"
	elseif c==1 then
		return "System"
	elseif c==2 then
		return "DLC"
	elseif c==3 then
		return "Patch"
	elseif c==4 then
		return "TWL"
	end
end
function OpenDirectory(text,archive_id)
	i=0
	if text == ".." then
		j=-2
		while string.sub(System.currentDirectory(),j,j) ~= "/" do
			j=j-1
		end
		System.currentDirectory(string.sub(System.currentDirectory(),1,j))
	else
		System.currentDirectory(System.currentDirectory()..text.."/")
	end
	files_table = listDirectory(System.currentDirectory())
	if System.currentDirectory() ~= "/" then
		local extra = {}
		extra.name = ".."
		extra.size = 0
		extra.directory = true
		table.insert(files_table,extra)
	end
end
while true do
	sm_index = 1
	base_y = 0
	Screen.waitVblankStart()
	Screen.refresh()
	Screen.clear(TOP_SCREEN)
	Screen.clear(BOTTOM_SCREEN)
	if mode == "SDMC" then
		for l, file in pairs(files_table) do
			if (base_y > 226) then
				break
			end
			if (l >= master_index) then
				if (l==p) then
					base_y2 = base_y
					if (base_y) == 0 then
						base_y = 2
					end
					Screen.fillRect(0,319,base_y-2,base_y2+12,selected_item,BOTTOM_SCREEN)
					color = selected_color
					if (base_y) == 2 then
						base_y = 0
					end
				else
					color = menu_color
				end
				CropPrint(0,base_y,file.name,color,BOTTOM_SCREEN)
				base_y = base_y + 15
			end
		end
		Screen.debugPrint(0,225,"SDMC listing",menu_color,TOP_SCREEN)
		if not files_table[p].directory then
			if not_extracted == true then
				cia_data = System.extractCIA(System.currentDirectory()..files_table[p].name)
			end
			Screen.debugPrint(0,0,"Title: "..cia_data.title,menu_color,TOP_SCREEN)
			Screen.debugPrint(0,15,"Unique ID: 0x"..string.sub(string.format('%02X',cia_data.unique_id),1,-3),menu_color,TOP_SCREEN)
		end
	else
		for l, file in pairs(cia_table) do
			if (base_y > 226) then
				break
			end
			if (l >= master_index) then
				if (l==p) then
					base_y2 = base_y
					if (base_y) == 0 then
						base_y = 2
					end
					Screen.fillRect(0,319,base_y-2,base_y2+12,selected_item,BOTTOM_SCREEN)
					color = selected_color
					if (base_y) == 2 then
						base_y = 0
					end
				else
					color = menu_color
				end
				CropPrint(0,base_y,"0x"..string.format('%02X',file.unique_id),color,BOTTOM_SCREEN)
				base_y = base_y + 15
			end
		end
		Screen.debugPrint(0,225,"Imported CIA listing",menu_color,TOP_SCREEN)
		Screen.debugPrint(0,0,"Unique ID: 0x"..string.sub(string.format('%02X',cia_table[p].unique_id),1,-3),menu_color,TOP_SCREEN)
		Screen.debugPrint(0,15,"Category: "..GetCategory(cia_table[p].category),menu_color,TOP_SCREEN)
		if (cia_table[p].platform == 3) then
			Screen.debugPrint(0,30,"Platform: DSi",menu_color,TOP_SCREEN)
		else
			Screen.debugPrint(0,30,"Platform: 3DS",menu_color,TOP_SCREEN)
		end
	end
	Controls.init()
	pad = Controls.read()
	if (Controls.check(pad,KEY_START)) then
		System.exit()
	end
	if (Controls.check(pad,KEY_A) and not Controls.check(oldpad,KEY_A)) then
		oldpad = KEY_A
		if (mode == "SDMC") then
			sm_index = 1
			if (files_table[p].directory) then
				OpenDirectory(files_table[p].name)
				p = 1
				master_index = 0
			else
				while true do
					Screen.waitVblankStart()
					Screen.refresh()
					Controls.init()
					pad = Controls.read()
					Screen.fillEmptyRect(60,260,50,82,selected_item,BOTTOM_SCREEN)
					Screen.fillRect(61,259,51,81,menu_color,BOTTOM_SCREEN)
					if (sm_index == 1) then
						Screen.fillRect(61,259,51,66,selected_color,BOTTOM_SCREEN)
						Screen.debugPrint(63,53,"Confirm",selected_item,BOTTOM_SCREEN)
						Screen.debugPrint(63,68,"Cancel",Color.new(0,0,0),BOTTOM_SCREEN)
						if (Controls.check(pad,KEY_DDOWN)) and not (Controls.check(oldpad,KEY_DDOWN)) then
							sm_index = 2
						elseif (Controls.check(pad,KEY_A)) and not (Controls.check(oldpad,KEY_A)) then
							System.installCIA(System.currentDirectory()..files_table[p].name)
							break
						end
					else
						Screen.fillRect(61,259,66,81,selected_color,BOTTOM_SCREEN)
						Screen.debugPrint(63,53,"Confirm",Color.new(0,0,0),BOTTOM_SCREEN)
						Screen.debugPrint(63,68,"Cancel",selected_item,BOTTOM_SCREEN)
						if (Controls.check(pad,KEY_DUP)) and not (Controls.check(oldpad,KEY_DUP)) then
							sm_index = 1
						elseif (Controls.check(pad,KEY_A)) and not (Controls.check(oldpad,KEY_A)) then
							break
						end
					end
					oldpad = pad
					Screen.flip()
				end
			end
		else
			while true do
				Screen.waitVblankStart()
				Screen.refresh()
				Controls.init()
				pad = Controls.read()
				Screen.fillEmptyRect(60,260,50,82,selected_item,BOTTOM_SCREEN)
				Screen.fillRect(61,259,51,81,menu_color,BOTTOM_SCREEN)
				if (sm_index == 1) then
					Screen.fillRect(61,259,51,66,selected_color,BOTTOM_SCREEN)
					Screen.debugPrint(63,53,"Confirm",selected_item,BOTTOM_SCREEN)
					Screen.debugPrint(63,68,"Cancel",Color.new(0,0,0),BOTTOM_SCREEN)
					if (Controls.check(pad,KEY_DDOWN)) and not (Controls.check(oldpad,KEY_DDOWN)) then
						sm_index = 2
					elseif (Controls.check(pad,KEY_A)) and not (Controls.check(oldpad,KEY_A)) then
						System.uninstallCIA(cia_table[p].delete_id)
						break
					end
				else
					Screen.fillRect(61,259,66,81,selected_color,BOTTOM_SCREEN)
					Screen.debugPrint(63,53,"Confirm",Color.new(0,0,0),BOTTOM_SCREEN)
					Screen.debugPrint(63,68,"Cancel",selected_item,BOTTOM_SCREEN)
					if (Controls.check(pad,KEY_DUP)) and not (Controls.check(oldpad,KEY_DUP)) then
						sm_index = 1
					elseif (Controls.check(pad,KEY_A)) and not (Controls.check(oldpad,KEY_A)) then
						break
					end
				end
				Screen.flip()
				oldpad = pad
			end
			cia_table = System.listCIA()
			if p > #cia_table then
				p = p - 1
			end
		end
	elseif (Controls.check(pad,KEY_DUP)) and not (Controls.check(oldpad,KEY_DUP)) then
		not_extracted = true
		p = p - 1
		if (p >= 16) then
			master_index = p - 15
		end
	elseif (Controls.check(pad,KEY_DDOWN)) and not (Controls.check(oldpad,KEY_DDOWN)) then
		not_extracted = true
		p = p + 1
		if (p >= 17) then
			master_index = p - 15
		end
	end
	if (p < 1) then
		if (mode == "SDMC") then
			p = #files_table
		else
			p = #cia_table
		end
		if (p >= 17) then
			master_index = p - 15
		end
	elseif ((p > #files_table) and (mode == "SDMC")) or ((p > #cia_table) and (mode == "CIA")) then
		master_index = 0
		p = 1
	end
	if (Controls.check(pad,KEY_SELECT)) and not (Controls.check(oldpad,KEY_SELECT)) then
		if mode=="CIA" then
			mode = "SDMC"
		else
			mode = "CIA"
			cia_table = System.listCIA()
		end
		p = 1
		master_index = 0
	end
	Screen.flip()
	oldpad = pad
end