module("@MODULE_NAME@", package.seeall)

require("bootpins")
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
		help="The expected strapping options.",
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

	-- Read the bootpins.
	atBootpins = bootpins.read(tPlugin)

	-- Compare the data with the expected values.
	local fOk = true
	if ulExpectedBootMode~=aBootpins.boot_mode then
		print(string.format("ERROR: the expected boot mode is 0x%08x, but 0x%08x was detected.", ulExpectedBootMode, aParameter[1]))
		fOk = false
	end
	if ulExpectedStrappingOptions~=aBootpins.strapping_options then
		print(string.format("ERROR: the expected strapping options are 0x%08x, but 0x%08x was detected.", ulExpectedStrappingOptions, aParameter[2]))
		fOk = false
	end
	if ulExpectedChipId~=aBootpins.chip_id then
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

