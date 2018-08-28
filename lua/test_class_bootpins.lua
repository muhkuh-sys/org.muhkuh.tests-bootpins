local class = require 'pl.class'
local TestClassBootpins = class()

function TestClassBootpins:_init(strTestName)
  self.parameters = require 'parameters'
  self.pl = require'pl.import_into'()
  local BootPins = require 'bootpins'
  self.bootpins = BootPins()

  self.CFG_strTestName = strTestName

  self.CFG_aParameterDefinitions = {
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
      validate=parameters.test_choice_single,
      constrains="NETX500,NETX100,NETX50,NETX10,NETX51A_NETX50_COMPATIBILITY_MODE,NETX51B_NETX50_COMPATIBILITY_MODE,NETX51A,NETX51B,NETX52A,NETX52B,NETX4000_RELAXED,NETX4000_FULL,NETX4000_SMALL,NETX90_MPW,NETX90"
    }
  }
end



function TestClassBootpins:run(aParameters, tLog)
  ----------------------------------------------------------------------
  --
  -- Parse the parameters and collect all options.
  --
  local ulExpectedBootMode         = tonumber(aParameters["expected_boot_mode"])
  local ulExpectedStrappingOptions = tonumber(aParameters["expected_strapping_options"])
  local strExpectedChipId          = aParameters["expected_chip_id"]
  local ulExpectedChipId = self.bootpins.atChipID[strExpectedChipId]
  if ulExpectedChipId==nil then
    error('Unknown chip ID: "%s"', strExpectedChipId)
  end

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
  local aBootPins = self.bootpins:read(tPlugin)
  local strDetectedChipId = self.bootpins.aulIdToChip[aBootPins.chip_id]
  if strDetectedChipId==nil then
    strDetectedChipId = 'invalid'
  end

  tLog.debug('Detected values:')
  tLog.debug('  boot mode: 0x%08x', aBootPins.boot_mode)
  tLog.debug('  strapping options: 0x%08x', aBootPins.strapping_options)
  tLog.debug('  chip id: %d (%s)', aBootPins.chip_id, strDetectedChipId)

  -- Compare the data with the expected values.
  local fOk = true
  if ulExpectedBootMode~=aBootPins.boot_mode then
    tLog.error('The expected boot mode is 0x%08x, but 0x%08x was detected.', ulExpectedBootMode, aBootPins.boot_mode)
    fOk = false
  end
  if ulExpectedStrappingOptions~=aBootPins.strapping_options then
    tLog.error('The expected strapping options are 0x%08x, but 0x%08x was detected.', ulExpectedStrappingOptions, aBootPins.strapping_options)
    fOk = false
  end
  if ulExpectedChipId~=aBootPins.chip_id then
    tLog.error('The expected chip ID is %d (%s), but %d (%s) was detected.', ulExpectedChipId, strExpectedChipId, aBootPins.chip_id, strDetectedChipId)
    fOk = false
  end
  if fOk~=true then
    error('The detected values do not match the expected data.')
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


return TestClassBootpins
