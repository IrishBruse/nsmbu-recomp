local scores = {
  0x1A86DD4C,
  0x1C54A0D8,
  0x1D04A718,
  0x1D04A748,
  0x1E9F1108,
}

local snap = nil
local dumped = false

local function guest_u32(addr)
  return nsmbu.guest.read_u32(addr)
end

local function guest_s32(addr)
  return nsmbu.guest.read_s32(addr)
end

local function dump_scores()
  for i = 1, #scores do
    local sa = scores[i]
    local parts = {}
    for off = -0x180, 0x40, 4 do
      local w = guest_u32(sa + off)
      if w and w <= 200 then
        parts[#parts + 1] = string.format("%+d=%d", off, w)
      end
    end
    nsmbu.log(string.format("score %08X=%u small=%s", sa, guest_u32(sa) or 0, table.concat(parts, " ")))
    for off = 0, 0x200, 4 do
      local a = sa - off
      if guest_u32(a) == 9 then
        local b0 = nsmbu.guest.read_u8(a - 4)
        local b1 = nsmbu.guest.read_u8(a - 3)
        local b2 = nsmbu.guest.read_u8(a - 2)
        local b3 = nsmbu.guest.read_u8(a - 1)
        local mode = guest_s32(a + 4)
        local coins30 = guest_s32(a + 0x30)
        local coins34 = guest_s32(a + 0x34)
        nsmbu.log(string.format(
          "  nine@-%03X bytes=%02X%02X%02X%02X mode=%s c30=%s c34=%s",
          off,
          b0 or 0,
          b1 or 0,
          b2 or 0,
          b3 or 0,
          tostring(mode),
          tostring(coins30),
          tostring(coins34)
        ))
      end
    end
  end
end

local function take_snap()
  local out = {}
  for addr = 0x10000000, 0x11000000 - 4, 4 do
    local w = guest_u32(addr)
    if w then
      local sec = math.floor(w / 4096)
      if sec >= 400 and sec <= 520 and (w % 4096) < 256 then
        out[addr] = w
      elseif w >= 400 and w <= 520 then
        out[addr] = w
      end
    end
  end
  return out
end

function nsmbu.on_logic_step(step)
  if step == 6800 and not dumped then
    dumped = true
    dump_scores()
    snap = take_snap()
    local n = 0
    for _ in pairs(snap) do
      n = n + 1
    end
    nsmbu.log("snap6800 n=" .. tostring(n))
  end
  if step == 7200 and snap then
    local dec = {}
    for addr, pw in pairs(snap) do
      local w = guest_u32(addr)
      if w and w < pw then
        local ps = pw
        local cs = w
        if pw > 700 then
          ps = math.floor(pw / 4096)
          cs = math.floor(w / 4096)
        end
        if cs < ps and (ps - cs) <= 20 then
          dec[#dec + 1] = string.format("%08X:%d->%d", addr, ps, cs)
        end
      end
    end
    table.sort(dec)
    nsmbu.log("dec count=" .. tostring(#dec))
    local i = 1
    while i <= #dec do
      local chunk = {}
      for j = i, math.min(i + 11, #dec) do
        chunk[#chunk + 1] = dec[j]
      end
      nsmbu.log("dec " .. table.concat(chunk, " "))
      i = i + 12
    end
  end
end
