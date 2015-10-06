module("@MODULE_NAME@", package.seeall)

require("parameters")

CFG_strTestName = "@TEST_NAME@"

CFG_aParameterDefinitions = {
	{
		name="expected_boot_mode",
		default=nil,
		help="The expected boot mode.",
		mandatory=true,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="expected_strapping_options",
		default=0,
		help="End address of the area to build the CRC32 for.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="expected_chip_id",
		default=nil,
		help="The expected chip ID.",
		mandatory=true,
		validate=parameters.test_uint32,
		constrains=nil
	}
}



function run(aParameters)
	----------------------------------------------------------------------
	--
	-- Parse the parameters and collect all options.
	--
	local ulExpectedBootMode         = tonumber(aParameters["expected_boot_mode"])
	local ulExpectedStrappingOptions = tonumber(aParameters["expected_strapping_options"])
	local ulExpectedChipId           = tonumber(aParameters["expected_chip_id"])
	
	----------------------------------------------------------------------
	--
	-- Open the connection to the netX.
	-- (or re-use an existing connection.)
	--
	local tPlugin = tester.getCommonPlugin()
	if tPlugin==nil then
		error("No plug-in selected, nothing to do!")
	end

	-- Get the binary for the ASIC.
	local tAsicTyp = tPlugin:GetChiptyp()
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

	-- Compare the data with the expected values.
	local fOk = true
	if ulExpectedBootMode~=aParameter[1] then
		print(string.format("ERROR: the expected boot mode is 0x%08x, but 0x%08x was detected.", ulExpectedBootMode, aParameter[1]))
		fOk = false
	end
	if ulExpectedStrappingOptions~=aParameter[2] then
		print(string.format("ERROR: the expected strapping options are 0x%08x, but 0x%08x was detected.", ulExpectedStrappingOptions, aParameter[2]))
		fOk = false
	end
	if ulExpectedChipId~=aParameter[1] then
		print(string.format("ERROR: the expected chip ID is 0x%08x, but 0x%08x was detected.", ulExpectedChipId, aParameter[3]))
		fOk = false
	end
	if fOk~=true then
		error("The detected values do not match the expected data.")
	end

	print("")
	print(" #######  ##    ## ")
	print("##     ## ##   ##  ")
	print("##     ## ##  ##   ")
	print("##     ## #####    ")
	print("##     ## ##  ##   ")
	print("##     ## ##   ##  ")
	print(" #######  ##    ## ")
	print("")
end

