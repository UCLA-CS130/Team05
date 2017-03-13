--[[
	A note about the LuaJIT implementation of this interface.
	Some liberties and simplifications were taken.  Primarily
	all the deprecated and obsolete functions were removed.  Similarly,
	all of the documentation has been removed to reduce the size
	of the file.

	In the cases where there were ifdef's around using double
	vs long long, the double was favored, and the references
	to long long were just removed.

	The hundreds of constants are not defined outright, rather,
	they are in tables, which are then used to generate the constants.
	This allows the programmer to decide whether they want the
	constants to be regular Lua number values, or if they should be
	static const int within the ffi.cdef section.  The CreateTokens()
	function can be used to generate whatever is desired.

	If it is desirable to reduce the size of the file, these tables
	can be removed once the desired values are generated.

	This is the most raw interface to SQLite3.  More friendly interfaces
	should be constructed in other wrapper files.
--]]

local ffi = require("ffi");
local langutils = require("langutils");

local constants = require("sqlite-ffi.sqlite3_constants");

function createEnumTokens(tokentable,enumname)
	enumname = enumname or "";

	local res = {};

	print(string.format("typedef enum %s {\n", enumname));
	for i,v in ipairs(tokentable) do
		print(string.format("    %s = %d,\n", v[1], v[2]));
	end
	print( "};");
end

function createTokenTable(tokentable)
	local res = {};

	for i,v in ipairs(tokentable) do
		res[v[1]] = v[2];
	end

	return res;
end

function createTokens(tokentable, driver)
	local str = driver(tokentable)
	local f = loadstring(str)
	f();
end

local printTable = function(tbl)
	for k,v in pairs(tbl) do
		print(string.format("%s = %d;",k,v));
	end
end

local errorcodes = createTokenTable(constants.sqliteerrorcodes);
local sqlitecodes = createTokenTable(constants.sqlitecodes);

langutils.importGlobal(errorcodes);

print([[
--This file is generated by running the
--gencodes.lua
--script.  If you want to alter this file, it is best
--to make the changes in that script and run it again.
	]]);
print("-- error codes")
printTable(errorcodes);
print("-- Codes");
printTable(sqlitecodes);

--createTokens(constants.sqlitecodes, GetLuaTokens);