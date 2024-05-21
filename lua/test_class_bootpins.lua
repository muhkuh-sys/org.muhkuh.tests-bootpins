local class = require 'pl.class'
local TestClass = require 'test_class'
local TestClassBootpins = class(TestClass)

function TestClassBootpins:_init(strTestName, uiTestCase, tLogWriter, strLogLevel)
  self:super(strTestName, uiTestCase, tLogWriter, strLogLevel)

  self.json = require 'dkjson'

  local BootPins = require 'bootpins'
  self.bootpins = BootPins()
  self.BootpinsOTP = require 'bootpins_otp'

  local P = self.P
  self:__parameter {
    P:P('plugin', 'A pattern for the plugin to use.'):
      required(false),

    P:P('plugin_options', 'Plugin options as a JSON object.'):
      required(false),

    P:U32('expected_boot_mode', 'The expected boot mode.'):
      required(false),

    P:U32('expected_strapping_options', 'The expected strapping options.'):
      required(false),

    P:MC('expected_chip_id', 'The expected chip ID.'):
      required(true):
      constraint(table.concat({
        'NETX500',
        'NETX100',
        'NETX50',
        'NETX10',
        'NETX51A_NETX50_COMPATIBILITY_MODE',
        'NETX51B_NETX50_COMPATIBILITY_MODE',
        'NETX51A',
        'NETX51B',
        'NETX52A',
        'NETX52B',
        'NETX4000_RELAXED',
        'NETX4000_FULL',
        'NETX4000_SMALL',
        'NETX90_MPW',
        'NETX90',
        'NETX90B',
        'NETX90BPHYR3',
        'NETX90C',
        'NETX90BPHYR2OR3'
      }, ',')),

    P:P('expected_otp_fuses', 'A file defining the expected OTP fuses.'):
      required(false),

    P:P('uid_blacklist', 'A blacklist for the chip UID. It must contain the exact UID in a line.'):
      required(false),

    P:SC('phy_is_critical',
         'Do not touch the PHY during the detection. A netX90B can not be distinguished further in this case.'):
      required(true):
      default('false'):
      constraint{'true','false'}
  }
end



function TestClassBootpins:read_otp_fuse_definition(strFile)
  local tLog = self.tLog
  tLog.debug('Reading OTP definition "%s".', strFile)

  local fOK = true
  local atExpectedValues = {}
  local tFile, strError = io.open(strFile, 'r')
  if tFile==nil then
    tLog.error('Failed to open file "%s" for reading: %s', strFile, strError)
    fOK = false
  else
    repeat
      local strLine = tFile:read('*l')
      if strLine~=nil then
        strLine = self.pl.stringx.strip(strLine)
        if strLine~='' and string.sub(strLine, 1)~='#' then
          local strName, strValue = string.match(strLine, '^([^%s]+)%s*=%s*([%xx]+)')
          if strName==nil then
            tLog.error('Failed to parse line: "%s"', strLine)
            fOK = false
          else
            local ulValue = tonumber(strValue)
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



function TestClassBootpins.__hexdump_reverse(strData)
  local astrHex = {}
  for uiPos=1,string.len(strData) do
    table.insert(astrHex, 1, string.format('%02x', string.byte(strData, uiPos)))
  end
  return table.concat(astrHex)
end



function TestClassBootpins:run()
  local atParameter = self.atParameter
  local tLog = self.tLog
  local json = self.json
  local pl = self.pl

  ----------------------------------------------------------------------
  --
  -- Parse the parameters and collect all options.
  --
  local strPluginPattern = atParameter['plugin']:get()
  local strPluginOptions = atParameter['plugin_options']:get()

  local ulExpectedBootMode         = atParameter["expected_boot_mode"]:get()
  local ulExpectedStrappingOptions = atParameter["expected_strapping_options"]:get()
  local astrExpectedChipId         = atParameter["expected_chip_id"]:get()
  local aulExpectedChipId = {}
  for _, strExpectedChipId in ipairs(astrExpectedChipId) do
    local ulExpectedChipId = self.bootpins.atChipID[strExpectedChipId]
    if ulExpectedChipId==nil then
      tLog.error('Unknown chip ID: "%s"', strExpectedChipId)
      error('Unknown chip ID.')
    end
    table.insert(aulExpectedChipId, ulExpectedChipId)
  end
  local strExpectedOtpFusesFile    = atParameter['expected_otp_fuses']:get()
  local atExpectedOtpFuses = nil
  if strExpectedOtpFusesFile~=nil then
    -- Read the OTP fuse definition line by line.
    atExpectedOtpFuses = self:read_otp_fuse_definition(strExpectedOtpFusesFile)
    if atExpectedOtpFuses==nil then
      local strError = string.format('Failed to parse the OTP fuse definition in "%s".', strExpectedOtpFusesFile)
      tLog.error(strError)
      error(strError)
    end
  end

  local atBlacklistLookup = nil
  local strBlacklistFilename = atParameter['uid_blacklist']:get()
  if strBlacklistFilename~=nil then
    atBlacklistLookup = {}

    -- Read the complete file.
    local strBlacklistData, strBlacklistError = pl.utils.readfile(strBlacklistFilename, false)
    if strBlacklistData==nil then
      local strError = string.format(
        'Failed to read the blacklist file "%s": %s',
        strBlacklistFilename,
        strBlacklistError
      )
      tLog.error(strError)
      error(strError)
    end
    -- Iterate over all lines.
    for strLine in pl.stringx.lines(strBlacklistData) do
      local strItem = pl.stringx.strip(strLine)
      if atBlacklistLookup[strItem]~=nil then
        tLog.debug('Dropping double blacklist entry "%s".', strItem)
      else
        atBlacklistLookup[strItem] = true
      end
    end
  end

  local fPhyIsCritical = (atParameter['phy_is_critical']:get() == 'true')

  ----------------------------------------------------------------------
  --
  -- Open the connection to the netX.
  -- (or re-use an existing connection.)
  --
  local atPluginOptions = {}
  if strPluginOptions~=nil then
    local tJson, uiPos, strJsonErr = json.decode(strPluginOptions)
    if tJson==nil then
      tLog.warning('Ignoring invalid plugin options. Error parsing the JSON: %d %s', uiPos, strJsonErr)
    else
      atPluginOptions = tJson
    end
  end
  local tPlugin = _G.tester:getCommonPlugin(strPluginPattern, atPluginOptions)
  if tPlugin==nil then
    local strPluginOptionsPretty = pl.pretty.write(atPluginOptions)
    local strError = string.format(
      'Failed to establish a connection to the netX with pattern "%s" and options "%s".',
      strPluginPattern,
      strPluginOptionsPretty
    )
    error(strError)
  end

  -- Create the flags.
  local ulFlags = 0
  if fPhyIsCritical then
    ulFlags = ulFlags + self.bootpins.atFlags.PHY_IS_CRITICAL
  end

  -- Read the bootpins.
  local aBootPins = self.bootpins:read(tPlugin, ulFlags)
  local strDetectedChipId = self.bootpins.aulIdToChip[aBootPins.chip_id]
  if strDetectedChipId==nil then
    strDetectedChipId = 'invalid'
  end

  local strUniqueId = nil
  if aBootPins.size_of_unique_id_in_bits>0 then
    strUniqueId = self.__hexdump_reverse(aBootPins.unique_id)
  end

  tLog.debug('Detected values:')
  tLog.debug('  boot mode: 0x%08x', aBootPins.boot_mode)
  tLog.debug('  strapping options: 0x%08x', aBootPins.strapping_options)
  tLog.debug('  chip id: %d (%s)', aBootPins.chip_id, strDetectedChipId)
  if strUniqueId==nil then
    tLog.debug('  no unique ID')
  else
    tLog.debug('  unique ID: %s', strUniqueId)
  end

  -- Compare the data with the expected values.
  local fOk = true
  local astrErrors = { 'The detected values do not match the expected data:' }
  if pl.tablex.find(aulExpectedChipId, aBootPins.chip_id)==nil then
    local astrMsg = {'The expected chip ID' }
    local sizChipIds = #aulExpectedChipId
    if sizChipIds==1 then
      table.insert(astrMsg, ' is ')
    else
      table.insert(astrMsg, 's are ')
    end
    local astrIDs = {}
    for iCnt=1,sizChipIds do
      table.insert(astrIDs, string.format('%d (%s)', aulExpectedChipId[iCnt], astrExpectedChipId[iCnt]))
    end
    table.insert(astrMsg, table.concat(astrIDs, ' or '))
    table.insert(astrMsg, string.format(', but %d (%s) was detected.', aBootPins.chip_id, strDetectedChipId))
    local strError = table.concat(astrMsg)
    table.insert(astrErrors, strError)
    fOk = false
  end
  if ulExpectedBootMode~=nil and ulExpectedBootMode~=aBootPins.boot_mode then
    local strError = string.format(
      'The expected boot mode is 0x%08x, but 0x%08x was detected.',
      ulExpectedBootMode,
      aBootPins.boot_mode
    )
    tLog.error(strError)
    table.insert(astrErrors, strError)
    fOk = false
  end
  if ulExpectedStrappingOptions~=nil and ulExpectedStrappingOptions~=aBootPins.strapping_options then
    local strError = string.format(
      'The expected strapping options are 0x%08x, but 0x%08x was detected.',
      ulExpectedStrappingOptions,
      aBootPins.strapping_options
    )
    table.insert(astrErrors, strError)
    fOk = false
  end
  if fOk~=true then
    error(table.concat(astrErrors, '\n'))
  end

  -- On the netX4000, read and compare the OTP fuses.
  if atExpectedOtpFuses~=nil then
    local tAsicTyp = tPlugin:GetChiptyp()
    if tAsicTyp==_G.romloader.ROMLOADER_CHIPTYP_NETX4000_RELAXED or
       tAsicTyp==_G.romloader.ROMLOADER_CHIPTYP_NETX4000_FULL or
       tAsicTyp==_G.romloader.ROMLOADER_CHIPTYP_NETX4100_SMALL then
      local bootpins_otp = self.BootpinsOTP(tLog)
      bootpins_otp:check(tPlugin, atExpectedOtpFuses, strExpectedOtpFusesFile)
    end
  end

  -- Is the UID part of the blacklist?
  if strUniqueId~=nil and atBlacklistLookup~=nil then
    if atBlacklistLookup[strUniqueId]==true then
      local strError = string.format('The chip UID %s is part of the blacklist.', strUniqueId)
      tLog.error(strError)
      error(strError)
    end
  end

  -- Log the chip ID and - if available - the UID.
  _G.tester:sendLogEvent('muhkuh.attribute.chipid', {
    chipid = strDetectedChipId,
    uid = strUniqueId
  })

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
