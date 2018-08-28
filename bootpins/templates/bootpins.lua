local class = require 'pl.class'
local BootPins = class()

function BootPins:_init()
  self.romloader = require 'romloader'
  self.tester = require 'tester'

  local atChipID = {
    ['unknown']                            = ${CHIPID_unknown},
    ['NETX500']                            = ${CHIPID_netX500},
    ['NETX100']                            = ${CHIPID_netX100},
    ['NETX50']                             = ${CHIPID_netX50},
    ['NETX10']                             = ${CHIPID_netX10},
    ['NETX51A_NETX50_COMPATIBILITY_MODE']  = ${CHIPID_netX51A_netX50_compatibility_mode},
    ['NETX51B_NETX50_COMPATIBILITY_MODE']  = ${CHIPID_netX51B_netX50_compatibility_mode},
    ['NETX51A']                            = ${CHIPID_netX51A},
    ['NETX51B']                            = ${CHIPID_netX51B},
    ['NETX52A']                            = ${CHIPID_netX52A},
    ['NETX52B']                            = ${CHIPID_netX52B},
    ['NETX4000_RELAXED']                   = ${CHIPID_netX4000_RELAXED},
    ['NETX4000_FULL']                      = ${CHIPID_netX4000_FULL},
    ['NETX4000_SMALL']                     = ${CHIPID_netX4000_SMALL},
    ['NETX90_MPW']                         = ${CHIPID_netX90_MPW},
    ['NETX90']                             = ${CHIPID_netX90}
  }
  self.atChipID = atChipID

  -- Build a reverse lookup table.
  local aulIdToChip = {}
  for strId, ulId in pairs(atChipID) do
    aulIdToChip[ulId] = strId
  end
  self.aulIdToChip = aulIdToChip

  self.astrBinaryName = {
    [romloader.ROMLOADER_CHIPTYP_NETX4000_RELAXED] = '4000',
    [romloader.ROMLOADER_CHIPTYP_NETX4000_FULL]    = '4000',
    [romloader.ROMLOADER_CHIPTYP_NETX4100_SMALL]   = '4000',
    [romloader.ROMLOADER_CHIPTYP_NETX500]          = '500',
    [romloader.ROMLOADER_CHIPTYP_NETX100]          = '500',
    [romloader.ROMLOADER_CHIPTYP_NETX90_MPW]       = '90_mpw',
--    [romloader.ROMLOADER_CHIPTYP_NETX90]           = '90',
    [romloader.ROMLOADER_CHIPTYP_NETX56]           = '56',
    [romloader.ROMLOADER_CHIPTYP_NETX56B]          = '56',
    [romloader.ROMLOADER_CHIPTYP_NETX50]           = '50',
    [romloader.ROMLOADER_CHIPTYP_NETX10]           = '10'
  }
end



-- Read the boot pins from the netX.
function BootPins:read(tPlugin)
  -- Get the binary for the ASIC.
  local tAsicTyp = tPlugin:GetChiptyp()
  local strBinary = self.astrBinaryName[tAsicTyp]
  if strBinary==nil then
    error('Unknown chiptyp!')
  end
  local strNetxBinary = string.format('netx/bootpins_netx%s.bin', strBinary)

  -- Download the binary, execute it and get the results back.
  local aParameter = {
    'OUTPUT',
    'OUTPUT',
    'OUTPUT'
  }
  local ulResult = tester.mbin_simple_run(nil, tPlugin, strNetxBinary, aParameter)
  if ulResult~=0 then
    error('The test failed with return code:' .. ulResult)
  end

  local atResult = {
    asic_typ = tAsicTyp,
    boot_mode = aParameter[1],
    strapping_options = aParameter[2],
    chip_id = aParameter[3]
  }

  return atResult
end

return BootPins
