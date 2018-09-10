local class = require 'pl.class'
local TestClassBootpins = class()

function TestClassBootpins:_init(strTestName)
  self.parameters = require 'parameters'
  self.pl = require'pl.import_into'()
  local BootPins = require 'bootpins'
  self.bootpins = BootPins()
  self.BootpinsOTP = require 'bootpins_otp'

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
    },
    {
      name="expected_otp_fuses",
      default=nil,
      help="A file defining the expected OTP fuses.",
      mandatory=false,
      validate=nil,
      constrains=nil
    }
  }
end



function TestClassBootpins:read_otp_fuse_definition(strFile, tLog)
  tLog.debug('Reading OTP definition "%s".', strFile)

  local fOK = true
  local atExpectedValues = {}
  local tFile, strError = io.open(strFile, 'r')
  if tFile==nil then
    tLog.error('Failed to open file "%s" for reading: %s', strFile, strError)
    fOK = false
  else
    repeat
      strLine = tFile:read('*l')
      if strLine~=nil then
        strLine = self.pl.stringx.strip(strLine)
        if strLine~='' and string.sub(strLine, 1)~='#' then
          strName, strValue = string.match(strLine, '^([^%s]+)%s*=%s*([%xx]+)')
          if strName==nil then
            tLog.error('Failed to parse line: "%s"', strLine)
            fOK = false
          else
            ulValue = tonumber(strValue)
            if ulValue==nil then
              tLog.error('Failed to parse the value in this line: "%s"', strLine)
              fOK = false
            elseif atExpectedValues[strName]~=nil then
              tLog.error('OTP value "%s" is defined more than once.', strName)
              fOK = false
            else
              atExpectedValues[strName] = ulValue
            end
          end
        end
      end
    until strLine==nil
  end

  if fOK~=true then
    atExpectedValues = nil
  end

  return atExpectedValues
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
    tLog.error('Unknown chip ID: "%s"', strExpectedChipId)
    error('Unknown chip ID.')
  end
  local strExpectedOtpFusesFile    = aParameters['expected_otp_fuses']
  local atExpectedOtpFuses = nil
  if strExpectedOtpFusesFile~=nil then
    -- Read the OTP fuse definition line by line.
    atExpectedOtpFuses = self:read_otp_fuse_definition(strExpectedOtpFusesFile, tLog)
    if atExpectedOtpFuses==nil then
      tLog.error('Failed to parse the OTP fuse definition in "%s".', strExpectedOtpFusesFile)
      error('Failed to parse the OTP fuse definition.')
    end
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

  -- On the netX4000, read and compare the OTP fuses.
  if atExpectedOtpFuses~=nil then
    local tAsicTyp = tPlugin:GetChiptyp()
    if tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX4000_RELAXED or tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX4000_FULL or tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX4100_SMALL then
      local bootpins_otp = self.BootpinsOTP(tLog)
      bootpins_otp:check(tPlugin, atExpectedOtpFuses)
    end
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
