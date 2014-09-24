--[[============================================================================
@file    ctcore.lua

@author  Daniel Zorychta

@brief   The core functions used by wizard

@note    Copyright (C) 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the  Free Software  Foundation;  either version 2 of the License, or
         any later version.

         This  program  is  distributed  in the hope that  it will be useful,
         but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         You  should  have received a copy  of the GNU General Public License
         along  with  this  program;  if not,  write  to  the  Free  Software
         Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


==============================================================================]]

--==============================================================================
-- EXTERNAL MODULES
--==============================================================================
require("wx")
xml = require("modules/xmlSimple").newParser()


--==============================================================================
-- PUBLIC OBJECTS
--==============================================================================
-- entire xml configuration mapped in the lua's table
config = xml:loadFile("config.xml").config

-- shared module variables
ct = {}
ct.fs = {}
ct.MAIN_WINDOW_NAME = config.tool.window.name:GetValue()
ct.WINDOW_X_SIZE    = tonumber(config.tool.window.xsize:GetValue())
ct.WINDOW_Y_SIZE    = tonumber(config.tool.window.ysize:GetValue())
ct.CONTROL_X_SIZE   = tonumber(config.tool.window.csize:GetValue())

ct.hexvalidator = wx.wxTextValidator(wx.wxFILTER_INCLUDE_CHAR_LIST)
ct.hexvalidator:SetIncludes({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "a", "b", "c", "d", "e", "f"})

ct.decvalidator = wx.wxTextValidator(wx.wxFILTER_INCLUDE_CHAR_LIST)
ct.decvalidator:SetIncludes({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"})

--==============================================================================
-- LOCAL OBJECTS
--==============================================================================
local FILETYPE_HEADER   = 0
local FILETYPE_MAKEFILE = 1


--==============================================================================
-- LOCAL FUNCTIONS
--==============================================================================
--------------------------------------------------------------------------------
-- @brief  Function try found specified key in the selected line
-- @param  line         line where key is finding
-- @param  filetype     type of file from line comes
-- @param  key          key to find
-- @return true if key was found, otherwise false
--------------------------------------------------------------------------------
local function line_found(line, filetype, key)
        if filetype == FILETYPE_HEADER then
                return line:match('^%s*#define%s+'..key..'%s*.*')

        elseif filetype == FILETYPE_MAKEFILE then
                return line:match('^%s*'..key..'%s*=%s*.*')
        end

        return false
end


--------------------------------------------------------------------------------
-- @brief  Function modify selected line by adding new value to the specified key
-- @param  line         line to modify
-- @param  filetype     type of file from line comes
-- @param  key          key to find
-- @param  value        value to modify
-- @return Modified line
--------------------------------------------------------------------------------
local function modify_line(line, filetype, key, value)
        if filetype == FILETYPE_HEADER then
                return "#define "..key.." "..value
        elseif filetype == FILETYPE_MAKEFILE then
                return key.."="..value
        end

        return line
end


--------------------------------------------------------------------------------
-- @brief  Function returns a value of selected key
-- @param  line         line from value is getting
-- @param  filetype     type of file from line comes
-- @param  key          key from value is getting
-- @return Value of selected key
--------------------------------------------------------------------------------
local function get_line_value(line, filetype, key)
        if filetype == FILETYPE_HEADER then
                local _, _, value = line:find("%s*#define%s+"..key.."%s+(.*)")
                return value
        elseif filetype == FILETYPE_MAKEFILE then
                local _, _, value = line:find("%s*"..key.."%s*=%s*(.*)")
                return value
        end

        return ""
end


--==============================================================================
-- PUBLIC FUNCTIONS
--==============================================================================
--------------------------------------------------------------------------------
-- @brief  Function loads configuration file (xml)
-- @return None
--------------------------------------------------------------------------------
function ct:reload_config_file()
        config = xml:loadFile("config.xml").config
end

--------------------------------------------------------------------------------
-- @brief  Function shows error dialog. Function kills entire wizard.
-- @param  title        window title
-- @param  caption      window caption
-- @param  parent       parent window
-- @return None
--------------------------------------------------------------------------------
function ct:show_error_msg(title, caption, parent)
        dialog = wx.wxMessageDialog(ifs(parent, parent, wx.NULL), caption, title, bit.bor(wx.wxOK, wx.wxICON_ERROR))
        dialog:ShowModal()
        wx.wxGetApp():ExitMainLoop()
        os.exit(0)
end


--------------------------------------------------------------------------------
-- @brief  Function shows info dialog
-- @param  title        window title
-- @param  caption      window caption
-- @param  parent       parent window
-- @return None
--------------------------------------------------------------------------------
function ct:show_info_msg(title, caption, parent)
        dialog = wx.wxMessageDialog(ifs(parent, parent, wx.NULL), caption, title, bit.bor(wx.wxOK, wx.wxICON_INFORMATION))
        dialog:ShowModal()
end


--------------------------------------------------------------------------------
-- @brief  Function shows question dialog
-- @param  title        window title
-- @param  caption      window caption
-- @param  buttons      wxWidgets button definitions to show
-- @param  parent       parent window
-- @return Selected button
--------------------------------------------------------------------------------
function ct:show_question_msg(title, caption, buttons, parent)
        dialog = wx.wxMessageDialog(ifs(parent, parent, wx.NULL), caption, title, bit.bor(buttons, wx.wxICON_QUESTION))
        return dialog:ShowModal()
end


--------------------------------------------------------------------------------
-- @brief  Function returns size of main window in form of table {xs, ys}
-- @param  None
-- @return Main window size if form of table {xs, ys}
--------------------------------------------------------------------------------
function ct:get_window_size()
        return ct.WINDOW_X_SIZE, ct.WINDOW_Y_SIZE
end


--------------------------------------------------------------------------------
-- @brief  Writes value of selected key
-- @param  keypath      key and path from xml configuration (table)
-- @param  value        value to write
-- @return On success true is returned. On error false is returned.
--------------------------------------------------------------------------------
function ct:key_write(keypath, value)
        -- check keypath
        if type(keypath) ~= "table" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): Invalid type of 'keypath <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        if keypath.path == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): Invalid type of 'keypath.path <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        if keypath.key == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): Invalid type of 'keypath.key <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        local filename = keypath.path:GetValue()
        local key      = keypath.key:GetValue()

        -- type check
        if type(filename) ~= "string" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): Invalid type of 'filename <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        if type(key) ~= "string" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): Invalid type of 'key <"..type(key)..">'\n"..debug.traceback())
                return false
        end

        if type(value) ~= "string" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): Invalid type of 'value <"..type(value)..">'\n"..debug.traceback())
                return false
        end

        -- read file
        local file = io.open(filename, "r")
        if file == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): "..filename..": Cannot open file specified\n"..debug.traceback())
                return false
        end

        -- file type check
        local filetype
        if filename:find(".h") or filename:find(".hpp") or filename:find(".hh") or filename:find(".hxx") then
                filetype = FILETYPE_HEADER
        elseif filename:find(".mk") or filename:find(".mak") or filename:find(".makefile") or filename:find("Makefile") or filename:find("makefile") then
                filetype = FILETYPE_MAKEFILE
        else
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Unknown file type\n"..debug.traceback())
                return false
        end

        -- key modifing
        local lines = {}
        local line_cnt = 1
        for line in file:lines() do

                if line_found(line, filetype, key) then
                        lines[#lines + 1] = modify_line(line, filetype, key, value)
                else
                        lines[#lines + 1] = line
                end

                line_cnt = line_cnt + 1
        end
        file:close()

        -- write the file.
        file = io.open(filename, "w")
        if file == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_write(): File write protected\n"..debug.traceback())
                return false
        end

        for i, line in ipairs(lines) do
                file:write(line, "\n")
        end
        file:close()

        return true
end


--------------------------------------------------------------------------------
-- @brief  Reads value of selected key
-- @param  keypath      key and path from xml configuration (table)
-- @return On success a value with form of string, otherwise nil.
--------------------------------------------------------------------------------
function ct:key_read(keypath)
        -- check keypath
        if type(keypath) ~= "table" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Invalid type of 'keypath <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        if keypath.path == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Invalid type of 'keypath.path <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        if keypath.key == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Invalid type of 'keypath.key <"..type(filename)..">'\n"..debug.traceback())
                return false
        end

        local filename = keypath.path:GetValue()
        local key      = keypath.key:GetValue()

        -- type check
        if type(filename) ~= "string" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Invalid type of 'filename'\n"..debug.traceback())
                return nil
        end

        if type(key) ~= "string" then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Invalid type of 'key'\n"..debug.traceback())
                return nil
        end

        -- read file
        local file = io.open(filename, "r")
        if file == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): "..filename..": Cannot open specified file\n"..debug.traceback())
                return nil
        end

        -- file type check
        local filetype
        if filename:find(".h") or filename:find(".hpp") or filename:find(".hh") or filename:find(".hxx") then
                filetype = FILETYPE_HEADER
        elseif filename:find(".mk") or filename:find(".mak") or filename:find(".makefile") or filename:find("Makefile") or filename:find("makefile") then
                filetype = FILETYPE_MAKEFILE
        else
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): Unknown file type\n"..debug.traceback())
                return nil
        end

        -- key finding
        local value = nil
        for line in file:lines() do
                if line_found(line, filetype, key) then
                        value = get_line_value(line, filetype, key)
                        if type(value) == "string" then
                                value = value:gsub("\r", "")
                        end

                        break
                end
        end
        file:close()

        if value == nil then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "key_read(): '"..key.."': key not found\n"..debug.traceback())
        end

        return value
end


--------------------------------------------------------------------------------
-- @brief  Returns an index of the string in the selected table
-- @param  tab      table of strings
-- @param  str      string to find
-- @return On success string index is returned (1..+). On error 0 is returned.
--------------------------------------------------------------------------------
function ct:get_string_index(tab, str)
        for i, s in ipairs(tab) do
                if s:match(str) then
                        return i
                end
        end

        return 0
end


--------------------------------------------------------------------------------
-- @brief  Function enables/disables selected module
-- @param  name     name of module (string)
-- @param  state    new state of module (bool)
-- @return None
--------------------------------------------------------------------------------
function ct:enable_module(name, state)
        local key1 = config.project.key["ENABLE_"..name:upper().."_H"]
        local key2 = config.project.key["ENABLE_"..name:upper().."_MK"]

        ct:key_write(key1, ct:bool_to_yes_no(state))
        ct:key_write(key2, ct:bool_to_yes_no(state))
end


--------------------------------------------------------------------------------
-- @brief  Gets module state (check configuration consistency)
-- @param  name     name of module (string)
-- @return Module state in form of bool
--------------------------------------------------------------------------------
function ct:get_module_state(name)
        local key1 = config.project.key["ENABLE_"..name:upper().."_H"]
        local key2 = config.project.key["ENABLE_"..name:upper().."_MK"]

        local key1_value = ct:key_read(key1)
        local key2_value = ct:key_read(key2)

        if key1_value ~= key2_value then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "Configuration inconsistency detected!\nValues of keys: ENABLE_"..name:upper().."_H and ENABLE_"..name:upper().."_MK are different!")
        end

        return ct:yes_no_to_bool(key1_value)
end


--------------------------------------------------------------------------------
-- @brief  Function converts yes/no strings to bool
-- @param  yes_no       yes/no string
-- @return yes_no converted to bool
--------------------------------------------------------------------------------
function ct:yes_no_to_bool(yes_no)
        if yes_no:match(config.project.def.YES:GetValue()) then
                return true
        else
                return false
        end
end


--------------------------------------------------------------------------------
-- @brief  Function converts bool to strings yes/no
-- @param  bool        value to convert
-- @return yes/no string
--------------------------------------------------------------------------------
function ct:bool_to_yes_no(bool)
        if bool then
                return config.project.def.YES:GetValue()
        else
                return config.project.def.NO:GetValue()
        end
end


--------------------------------------------------------------------------------
-- @brief  Function returns index of selected CPU
-- @param  cpu_arch     CPU architecture
-- @param  cpu_name     name of CPU
-- @return On success return CPU index from xml configuration. On error 0.
--------------------------------------------------------------------------------
function ct:get_cpu_index(cpu_arch, cpu_name)
        for i = 1, config.arch[cpu_arch].cpulist:NumChildren() do
                if config.arch[cpu_arch].cpulist:Children()[i].name:GetValue() == cpu_name then
                        return i
                end
        end

        return 0
end


--------------------------------------------------------------------------------
-- @brief Short verion of if statement
-- @param expr      test expression
-- @param ontrue    return value if expr is true
-- @param onfalse   return value if expr is false
-- @return Return ontrue if expr is true, otherwise onfalse is returned.
--------------------------------------------------------------------------------
function ifs(expr, ontrue, onfalse)
        if expr then
                return ontrue
        else
                return onfalse
        end
end


--------------------------------------------------------------------------------
-- @brief  Function return list of priorities for selected architecture
-- @param  cpu_arch     selected CPU architecture
-- @return List of priorities. Item format: {["name"] = name, ["value"] = value}
--------------------------------------------------------------------------------
function ct:get_priority_list(cpu_arch)
        local list = {}

        for i = 1, config.arch[cpu_arch].priorities:NumChildren() do
                local item = {}
                item.name  = config.arch[cpu_arch].priorities.priority[i].name:GetValue()
                item.value = config.arch[cpu_arch].priorities.priority[i].value:GetValue()
                list[#list + 1] = item
        end

        return list
end


--------------------------------------------------------------------------------
-- @brief Function round value
-- @param value         value to round
-- @param precision     number of digits
-- @return Rounded value
--------------------------------------------------------------------------------
function math.round(value, precision)
        return math.floor(value * 10^(precision or 0)) / 10^(precision or 0)
end


--------------------------------------------------------------------------------
-- @brief  Check time value and set unit
-- @param  time         time [s]
-- @return string with calculated time and adjusted unit
--------------------------------------------------------------------------------
function ct:print_time(time)
        if time < 1e-6 then
                return math.round(time*1e9, 3).." ns"
        elseif time < 1e-3 then
                return math.round(time*1e6, 3).." us"
        elseif time < 1 then
                return math.round(time*1e3, 3).." ms"
        else
                return time.." s"
        end
end


--------------------------------------------------------------------------------
-- @brief  Calculate frequency unit
-- @param  freq         frequency
-- @return string of frequency with adjusted unit
--------------------------------------------------------------------------------
function ct:print_freq(freq)
        if freq < 1e3 then
                return math.round(freq, 3).." Hz"
        elseif freq < 1e6 then
                return math.round(freq/1e3, 3).." kHz"
        else
                return math.round(freq/1e6, 3).." MHz"
        end
end


--------------------------------------------------------------------------------
-- @brief  Found line by using regular expressions
-- @param  filename     file where line will be search
-- @param  startline    start line
-- @param  regex        regular expression (Lua's version)
-- @return Number of line where expression was found or 0 when not found
--------------------------------------------------------------------------------
function ct:find_line(filename, startline, regex)
        assert(type(filename) == "string", "find_line(): filename is not the string type")
        assert(type(startline) == "number", "find_line(): startline is not the number type")
        assert(type(regex) == "string", "find_line(): regex is not the string type")

        file = io.open(filename, "r")
        assert(file, "find_line(): file does not exist")

        file:seek("set", 0)

        local n = 1

        for line in file:lines() do
                if line:match(regex) and n >= startline then
                        file:close()
                        return n
                end

                n = n + 1
        end

        file:close()
        return 0
end


--------------------------------------------------------------------------------
-- @brief  Insert new line at selected position. If position does not exist then
--         line is not placed.
-- @param  filename     file where line will be inserted
-- @param  lineno       line number where a new line shall be inserted
-- @param  newline      a new line
-- @return On success true is returned, otherwise false
--------------------------------------------------------------------------------
function ct:insert_line(filename, lineno, newline)
        assert(type(filename) == "string", "insert_line(): filename is not the string type")
        assert(type(lineno) == "number", "insert_line(): lineno is not the number type")
        assert(type(newline) == "string", "insert_line(): newline is not the string type")

        file = io.open(filename, "r+")
        assert(file, "insert_line(): file does not exist")

        file:seek("set", 0)

        local data = {}
        for line in file:lines() do
                table.insert(data, line)
        end

        if lineno > #data then
                return false
        else
                table.insert(data, lineno, newline)
        end

        file:seek("set", 0)
        for i, line in ipairs(data) do
                file:write(line, "\n")
        end

        file:close()

        return true
end


--------------------------------------------------------------------------------
-- @brief  Remove selected line from file
-- @param  filename     file where line will be removed
-- @param  lineno       number of line that will be removed
-- @return On success true is returned, otherwise false
--------------------------------------------------------------------------------
function ct:remove_line(filename, lineno)
        assert(type(filename) == "string", "remove_line(): filename is not the string type")
        assert(type(lineno) == "number", "remove_line(): lineno is not the number type")

        file = io.open(filename, "r")
        assert(file, "remove_line(): '"..filename.."' file does not exist")

        file:seek("set", 0)

        local data = {}
        for line in file:lines() do
                table.insert(data, line)
        end

        if lineno > #data then
                return false
        else
                table.remove(data, lineno)
        end

        file:close()

        file = io.open(filename, "w")
        assert(file, "remove_line(): '"..filename.."' no permissions to write file")

        file:seek("set", 0)
        for i, line in ipairs(data) do
                file:write(line, "\n")
        end

        file:close()

        return true
end


--------------------------------------------------------------------------------
-- @brief  Remove selected line from file
-- @param  template_path        path to the template file
-- @param  destination_path     path where modified template will be saved
-- @param  replace_tags         tag translation table {{.tag = "", .to = ""}, ...}
-- @return Number of repleaced tags
--------------------------------------------------------------------------------
function ct:apply_template(template_path, destination_path, replace_tags)
        assert(type(template_path) == "string", "apply_template(): template_path is not the string type")
        assert(type(destination_path) == "string", "apply_template(): destination_path is not the string type")
        assert(type(replace_tags) == "table", "apply_template(): replace_tags is not the table type")

        local n = 0

        template = io.open(template_path, "r")
        if not template then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "apply_template(): file '"..template_path.."' does not exist.\n"..debug.traceback())
                return n
        end

        dest = io.open(destination_path, "w")
        if not dest then
                ct:show_error_msg(ct.MAIN_WINDOW_NAME, "apply_template(): file '"..destination_path.."' does not exist.\n"..debug.traceback())
                template:close()
                return n
        end

        template:seek("set", 0)
        dest:seek("set", 0)

        for line in template:lines() do
                for _, t in pairs(replace_tags) do
                        line = line:gsub(t.tag, t.to)
                end

                dest:write(line, "\n")
        end

        template:close()
        dest:close()

        return n
end


--------------------------------------------------------------------------------
-- @brief  Check if file or directory exists
-- @param  name     file/directory name
-- @return Retrun true if file/dir exists, otherwise false
--------------------------------------------------------------------------------
function ct.fs:exists(name)
        if type(name)~="string" then
                return false
        end

        return os.rename(name, name) and true or false
end


--------------------------------------------------------------------------------
-- @brief  Check if path is a file
-- @param  name     path
-- @return Retrun true if path is file, otherwise false
--------------------------------------------------------------------------------
function ct.fs:is_file(name)
        if type(name)~="string" then
                return false
        end

        if not ct.fs:exists(name) then
                return false
        end

        local f = io.open(name, "r")
        if f then
                if f:read(1) then
                        f:close()
                        return true
                else
                        f:close()
                        return false
                end
        end

        return false
end


--------------------------------------------------------------------------------
-- @brief  Check if path is a directory
-- @param  name     path
-- @return Retrun true if path is directory, otherwise false
--------------------------------------------------------------------------------
function ct.fs:is_dir(name)
        return (ct.fs:exists(name) and not ct.fs:is_file(name))
end


--------------------------------------------------------------------------------
-- @brief  Create a new directory (can create an entire path)
-- @param  name     path
-- @return Retrun true if directory is created, otherwise false
--------------------------------------------------------------------------------
function ct.fs:mkdir(name)
        if type(name)~="string" then
                return false
        end

        if ct.fs:is_dir(name) then
                return true
        else
                os.execute("bash -c '/bin/mkdir -p "..name.."'")
                return ct.fs:is_dir(name)
        end
end


--------------------------------------------------------------------------------
-- @brief  Remove selected file or directory
-- @param  name     path
-- @return Retrun true if object is removed, otherwise false
--------------------------------------------------------------------------------
function ct.fs:remove(name)
        if type(name)~="string" then
                return false
        end

        os.execute("bash -c '/bin/rm -rf "..name.."'")
        return true
end