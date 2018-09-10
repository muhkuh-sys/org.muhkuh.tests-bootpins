local class = require 'pl.class'
local BootpinsOTP = class()


function BootpinsOTP:_init(tLog)
  self.bit = require 'bit'
  self.vstruct = require 'vstruct'
  self.pl = require'pl.import_into'()

  self.tLog = tLog
end



-- A short function to count all bits set to 1.
function BootpinsOTP:__cb(uiBitSize, ...)
  -- Loop over all variadic arguments and count the bits which are '1'.
  local uiElements = select('#', ...)
  local uiBitCnt = 0
  for uiCnt=1, uiElements do
    local ulValue = select(uiCnt, ...)
    while ulValue~=0 do
      if self.bit.band(ulValue, 1)==1 then
        uiBitCnt = uiBitCnt + 1
      end
      ulValue = self.bit.rshift(ulValue, 1)
    end
  end

  -- Invert the bit count.
  local uiBitCntInv = self.bit.bxor(uiBitCnt, self.bit.lshift(1, uiBitSize) - 1)

  -- Combine the non-inverted and the inverted part.
  local ulBitCntBoth = self.bit.bor(uiBitCnt, self.bit.lshift(uiBitCntInv, uiBitSize))

  return ulBitCntBoth
end



function BootpinsOTP:__bm(ulValue)
  -- The checksum of the bootmode pins is special.
  -- Invert and mask bits 2-4 and shift them down to 0.
  return self.bit.rshift(self.bit.band(self.bit.bxor(ulValue, 0x1c), 0x1c), 2)
end



function BootpinsOTP:check(tPlugin, atExpectedValues)
--  local tester = require 'tester_cli'

  -- Read the chip ID.
  local strChipID = tPlugin:read_image(0xf80000b0, 0x10, tester.callback_progress, 0x10)

  -- Read all OTP fuses.
  local strOTPFuses = tPlugin:read_image(0xf80000c0, 0x28, tester.callback_progress, 0x28)

  -- Parse the chip ID.
  local strChipIDFormat = [[
    4*u4
  ]]
  local tChipID = self.vstruct.read(strChipIDFormat, strChipID)


  -- Parse the OTP fuses.
  local strOTPFormat = [[
HW_FUSES:{
  MODE:{[1|
    package:u1
    cpu0:u1
    cpu1:u1
    usb2jtag:u1
    reserved:u4]}

  CLOCK_MASK:{[1|
    USB20:u1
    PCIe:u1
    SDIO:u1
    GMAC:u1
    SWITCH:u1
    TES:u1
    DDR:u1
    reserved:u1]}

  NETX_HW_FUSE:{[2|
    coresight_apbap_master:u1
    coreshight_ahbap_master:u1
    coresight_non_invasive_debugging:u1
    coresight_secure_invasive_debugging:u1
    coresight_secure_non_invasive_debugging:u1
    cortex_r7_invasive_debugging:u1
    cortex_r7_non_invasive_debugging:u1
    cortex_a9_invasive_debugging:u1
    cortex_a9_non_invasive_debugging:u1
    cortex_a9_secure_invasive_debugging:u1
    pl310l2_cache_cortex_a9_secure_non_invasive_debugging:u1
    netx_sysdebug:u1
    reserved12:u1
    cortex_r7_global_debug_disable:u1
    cortex_a9_global_debug_disable:u1
    netx_sysdebug_global_debug_disable:u1]}
}

ulChipIDProtection:u2

tRSA:{<[2|
  ulChipType:u3
  ulInvalidKeys:u6
  ulKeySelect:u7]}
ulRsaProtection:u1

tLic:{<[1|
  ulReserved4C:u4
  ulErr:u4]}

tLicenses:{<[17|
  ulSoftwareLicensesProtection:u12
  ulSoftwareLicenses:u32
  ulMasterLicensesProtection:u12
  ulMasterLicenses:u32
  ulCustomerIDLicenseFlagsProtection:u12
  ulLicenseFlags:u16
  ulCustomerIDFlags:u4
  ulCustomerID:u16]}

tID:{<[11|
  ulProdIDProtection:u12
  ulProdID:u32
  ulManIDProtection:u12
  ulManID:u32]}


tBootMode:{<[1|
  ulBootModeProtection:u3
  ulBootMode:u5]}

ulSecMode:u1
  ]]
  local tOTP = self.vstruct.read(strOTPFormat, strOTPFuses)


  -- TODO: print all decoded values.


  -- Be optimistic.
  fOK = true

  -- Check all protection fields.
  local atProtection = {
    ['chip ID'] =     { tOTP.ulChipIDProtection,                           atExpectedValues['ulChipIDProtection'],                           self:__cb(8, tChipID[1], tChipID[2], tChipID[3], tChipID[4]) },
    ['RSA'] =         { tOTP.ulRsaProtection,                              atExpectedValues['ulRsaProtection'],                              self:__cb(4, tOTP.tRSA.ulKeySelect, tOTP.tRSA.ulInvalidKeys, tOTP.tRSA.ulChipType) },
    ['customer ID'] = { tOTP.tLicenses.ulCustomerIDLicenseFlagsProtection, atExpectedValues['tLicenses.ulCustomerIDLicenseFlagsProtection'], self:__cb(6, tOTP.tLicenses.ulCustomerID, tOTP.tLicenses.ulCustomerIDFlags, tOTP.tLicenses.ulLicenseFlags) },
    ['master lic'] =  { tOTP.tLicenses.ulMasterLicensesProtection,         atExpectedValues['tLicenses.ulMasterLicensesProtection'],         self:__cb(6, tOTP.tLicenses.ulMasterLicenses) },
    ['sw lic'] =      { tOTP.tLicenses.ulSoftwareLicensesProtection,       atExpectedValues['tLicenses.ulSoftwareLicensesProtection'],       self:__cb(6, tOTP.tLicenses.ulSoftwareLicenses) },
    ['man id'] =      { tOTP.tID.ulManIDProtection,                        atExpectedValues['tID.ulManIDProtection'],                        self:__cb(6, tOTP.tID.ulManID) },
    ['prod id'] =     { tOTP.tID.ulProdIDProtection,                       atExpectedValues['tID.ulProdIDProtection'],                       self:__cb(6, tOTP.tID.ulProdID) },
    ['boot mode'] =   { tOTP.tBootMode.ulBootModeProtection,               atExpectedValues['tBootMode.ulBootModeProtection'],               self:__bm(tOTP.tBootMode.ulBootMode) }
  }
  for strTestName, atTest in pairs(atProtection) do
    local ulProtection = atTest[1]
    local ulOverride = atTest[2]
    local ulCalculated = atTest[3]

    if ulOverride~=nil then
      if ulProtection==ulOverride then
        self.tLog.debug('Field "%s" ok (override)', strTestName)
      else
        self.tLog.error('Field "%s" failed (override).', strTestName)
        self.tLog.debug('  Protection: 0x%08x,  Override: 0x%08x', ulProtection, ulOverride)
        fOK = false
      end

    else
      if ulProtection==ulCalculated then
        self.tLog.debug('Field "%s" ok', strTestName)
      else
        self.tLog.error('Field "%s" failed.', strTestName)
        self.tLog.debug('Protection: 0x%08x,  Expected: 0x%08x', ulProtection, ulCalculated)
        fOK = false
      end
    end
  end

  -- Compare all expected values with the read data.
  for strKey, ulValueExpected in pairs(atExpectedValues) do
    -- Split the key by dots.
    local atPath = self.pl.stringx.split(strKey, '.')
    local tElem = tOTP
    local strPathCnt = nil
    for _, strPath in ipairs(atPath) do
      -- Get the current path for error messages.
      if strPathCnt==nil then
        strPathCnt = strPath
      else
        strPathCnt = strPathCnt .. '.' .. strPath
      end

      -- The current element must be a table.
      if type(tElem)~='table' then
        self.tLog.error('The path "%s" does not point to a table.', strPathCnt)
        error('Invalid path in OTP definition.')
      end

      -- Does the new path element exist on the current level?
      local tNewElem = tElem[strPath]
      if tNewElem==nil then
        self.tLog.error('The path "%s" does not exist.', strPathCnt)
        error('Invalid path in OTP definition.')
      end

      -- Accept the new path.
      tElem = tNewElem
    end

    -- The complete path must point to a number.
    if type(tElem)~='number' then
      self.tLog.error('The path "%s" does not point to a number.', strKey)
      error('Invalid path in OTP definition.')
    end
    local ulValueIs = tElem

    if ulValueIs==ulValueExpected then
      self.tLog.debug('Comparing path %s = %s -> OK', strKey, ulValueExpected)
    else
      self.tLog.error('The path %s has the value 0x%08x, but 0x%08x was expected.', strKey, ulValueIs, ulValueExpected)
      fOK = false
    end
  end

  if fOK~=true then
    error('The OTP fuses do not match the definition.')
  end
end

return BootpinsOTP
