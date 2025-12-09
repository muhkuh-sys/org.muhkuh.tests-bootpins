local class = require 'pl.class'
local BootPins = class()

function BootPins:_init()
  self.romloader = require 'romloader'

  local atChipID = {
    ['unknown']                            = XX_CHIPID_unknown_XX,
    ['NETX500']                            = XX_CHIPID_netX500_XX,
    ['NETX100']                            = XX_CHIPID_netX100_XX,
    ['NETX50']                             = XX_CHIPID_netX50_XX,
    ['NETX10']                             = XX_CHIPID_netX10_XX,
    ['NETX51A_NETX50_COMPATIBILITY_MODE']  = XX_CHIPID_netX51A_netX50_compatibility_mode_XX,
    ['NETX51B_NETX50_COMPATIBILITY_MODE']  = XX_CHIPID_netX51B_netX50_compatibility_mode_XX,
    ['NETX51A']                            = XX_CHIPID_netX51A_XX,
    ['NETX51B']                            = XX_CHIPID_netX51B_XX,
    ['NETX52A']                            = XX_CHIPID_netX52A_XX,
    ['NETX52B']                            = XX_CHIPID_netX52B_XX,
    ['NETX4000_RELAXED']                   = XX_CHIPID_netX4000_RELAXED_XX,
    ['NETX4000_FULL']                      = XX_CHIPID_netX4000_FULL_XX,
    ['NETX4000_SMALL']                     = XX_CHIPID_netX4000_SMALL_XX,
    ['NETX90_MPW']                         = XX_CHIPID_netX90_MPW_XX,
    ['NETX90']                             = XX_CHIPID_netX90_XX,
    ['NETX90B']                            = XX_CHIPID_netX90B_XX,
    ['NETX90BPHYR3']                       = XX_CHIPID_netX90BPhyR3_XX,
    ['NETX90C']                            = XX_CHIPID_netX90C_XX,
    ['NETX90BPHYR2OR3']                    = XX_CHIPID_netX90BPhyR2or3_XX
  }
  self.atChipID = atChipID

  -- Build a reverse lookup table.
  local aulIdToChip = {}
  for strId, ulId in pairs(atChipID) do
    aulIdToChip[ulId] = strId
  end
  self.aulIdToChip = aulIdToChip

  self.atFlags = {
    ['PHY_IS_CRITICAL']                 = XX_BOOTPINS_FLAGS_PHY_IS_CRITICAL_XX
  }

  local romloader = require 'romloader'
  self.astrBinaryName = {
    [romloader.ROMLOADER_CHIPTYP_NETX4000_RELAXED] = '4000',
    [romloader.ROMLOADER_CHIPTYP_NETX4000_FULL]    = '4000',
    [romloader.ROMLOADER_CHIPTYP_NETX4100_SMALL]   = '4000',
    [romloader.ROMLOADER_CHIPTYP_NETX500]          = '500',
    [romloader.ROMLOADER_CHIPTYP_NETX100]          = '500',
    [romloader.ROMLOADER_CHIPTYP_NETX90_MPW]       = '90_mpw',
    [romloader.ROMLOADER_CHIPTYP_NETX90]           = '90',
    [romloader.ROMLOADER_CHIPTYP_NETX90B]          = '90',
    [romloader.ROMLOADER_CHIPTYP_NETX56]           = '56',
    [romloader.ROMLOADER_CHIPTYP_NETX56B]          = '56',
    [romloader.ROMLOADER_CHIPTYP_NETX50]           = '50',
    [romloader.ROMLOADER_CHIPTYP_NETX10]           = '10'
--    [romloader.ROMLOADER_CHIPTYP_NETIOLA]          = 'IOL',
--    [romloader.ROMLOADER_CHIPTYP_NETIOLB]          = 'IOL'
  }
end



-- Read the boot pins from the netX.
function BootPins:read(tPlugin, ulFlags)
  ulFlags = ulFlags or 0

  -- Get the binary for the ASIC.
  local tAsicTyp = tPlugin:GetChiptyp()
  local strBinary = self.astrBinaryName[tAsicTyp]
  if strBinary==nil then
    error('Unknown chiptyp!')
  end
  local strNetxBinary = string.format('netx/bootpins_netx%s.bin', strBinary)

  -- Download the binary, execute it and get the results back.
  local aParameter = {
    ulFlags,
    'OUTPUT',
    'OUTPUT',
    'OUTPUT',
    'OUTPUT',
    'OUTPUT'
  }
  local tester = _G.tester
  local aAttr = tester:mbin_open(strNetxBinary, tPlugin)
  tester:mbin_debug(aAttr)
  tester:mbin_write(tPlugin, aAttr)
  tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
  local ulResult = tester:mbin_execute(tPlugin, aAttr, aParameter)
  if ulResult~=0 then
    error('The test failed with return code:' .. ulResult)
  end

  -- Read the unique ID if there is one.
  local sizUniqueIdInBits = aParameter[5]
  local strUniqueId = ''
  if sizUniqueIdInBits>0 then
    -- Check for an upper limit.
    if sizUniqueIdInBits>2048 then
      error('Cowardly refusing to read more than 2048 bits.')
    end
    -- Get the size of the unique ID in bytes.
    local sizUniqueId = math.ceil(sizUniqueIdInBits / 8)

    -- Read the unique ID.
    strUniqueId = tester:stdRead(tPlugin, aAttr.ulParameterStartAddress+0x0c+0x14, sizUniqueId)
  end

  local atResult = {
    asic_typ = tAsicTyp,
    boot_mode = aParameter[2],
    strapping_options = aParameter[3],
    chip_id = aParameter[4],
    size_of_unique_id_in_bits = sizUniqueIdInBits,
    unique_id = strUniqueId
  }

  return atResult
end

return BootPins
