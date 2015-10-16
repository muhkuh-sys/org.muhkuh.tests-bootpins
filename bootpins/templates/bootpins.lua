module("bootpins", package.seeall)

require("romloader")
require("tester")


CHIPID_unknown                              = ${CHIPID_unknown}
CHIPID_netX500                              = ${CHIPID_netX500}
CHIPID_netX100                              = ${CHIPID_netX100}
CHIPID_netX50                               = ${CHIPID_netX50}
CHIPID_netX10                               = ${CHIPID_netX10}
CHIPID_netX51A_netX50_compatibility_mode    = ${CHIPID_netX51A_netX50_compatibility_mode}
CHIPID_netX51B_netX50_compatibility_mode    = ${CHIPID_netX51B_netX50_compatibility_mode}
CHIPID_netX51A                              = ${CHIPID_netX51A}
CHIPID_netX51B                              = ${CHIPID_netX51B}
CHIPID_netX52A                              = ${CHIPID_netX52A}
CHIPID_netX52B                              = ${CHIPID_netX52B}


-- Read the boot pins from the netX.
function read(tPlugin)
	-- Get the binary for the ASIC.
	local tAsicTyp = tPlugin:GetChiptyp()
	local uiAsic = nil
	if tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX50 then
		uiAsic = 50
	elseif tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX100 or tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX500 then
		uiAsic = 500
	elseif tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX10 then
		uiAsic = 10
	elseif tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX56 or tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX56B then
		uiAsic = 56
	else
		error("Unknown chiptyp!")
	end
	local strNetxBinary = string.format("netx/bootpins_netx%d.bin", uiAsic)
	
	-- Download the binary, execute it and get the results back.
	local aParameter = {
		"OUTPUT",
		"OUTPUT",
		"OUTPUT"
	}
	local ulResult = tester.mbin_simple_run(nil, tPlugin, strNetxBinary, aParameter)
	if ulResult~=0 then
		error("The test failed with return code:" .. ulResult)
	end
	
	local atResult = {
		asic_typ = tAsicTyp,
		boot_mode = aParameter[1],
		strapping_options = aParameter[2],
		chip_id = aParameter[3]
	}
	
	return atResult
end


-- Print the boot mode pins.
--function pretty_print(atData)
-- TODO: print something 
--end


