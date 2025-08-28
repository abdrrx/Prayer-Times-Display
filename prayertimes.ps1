# prayertimes.ps1 — fetches today's prayer times and streams to Arduino
# 1) Change these:
$comPort = "COM6"                    # your Arduino port (Arduino IDE → Tools → Port)
$city    = "London"
$country = "United Kingdom"
$method  = 3                         # 2=ISNA, 3=MWL, 4=Umm Al-Qura, 5=Egyptian, etc.

# 2) Open serial
$baud = 9600
$sp = [System.IO.Ports.SerialPort]::new($comPort, $baud, 'None', 8, 'One')
$sp.NewLine = "`n"
$sp.Open()

function Send-Line($s){ if($sp.IsOpen){ $sp.WriteLine($s) } }

# 3) Fetch today's timings (Aladhan API)
# Docs: api.aladhan.com — timingsByCity
$uri = "https://api.aladhan.com/v1/timingsByCity?city=$([Uri]::EscapeDataString($city))&country=$([Uri]::EscapeDataString($country))&method=$method"
try {
  $resp = Invoke-RestMethod -Uri $uri -TimeoutSec 10 -ErrorAction Stop
} catch {
  Write-Host "Failed to fetch timings: $($_.Exception.Message)"
  if($sp.IsOpen){ $sp.Close() }
  exit 1
}

$tz   = $resp.data.meta.timezone
$tim  = $resp.data.timings
# Normalize to 24h HH:MM without seconds
function HHMM($str){
  if(-not $str){ return "00:00" }
  # Remove possible "(BST)" etc., then take first 5 chars
  $s = ($str -replace '[^\d:]', '')
  if($s.Length -ge 5){ return $s.Substring(0,5) } else { return "00:00" }
}

$fajr    = HHMM $tim.Fajr
$sunrise = HHMM $tim.Sunrise
$dhuhr   = HHMM $tim.Dhuhr
$asr     = HHMM $tim.Asr
$maghrib = HHMM $tim.Maghrib
$isha    = HHMM $tim.Isha

# 4) Send the PRAYERS line to Arduino (once)
# Format: PRAYERS|Fajr|HH:MM|Sunrise|HH:MM|Dhuhr|HH:MM|Asr|HH:MM|Maghrib|HH:MM|Isha|HH:MM
$line = "PRAYERS|Fajr|$fajr|Sunrise|$sunrise|Dhuhr|$dhuhr|Asr|$asr|Maghrib|$maghrib|Isha|$isha"
Send-Line $line

Write-Host "Sent prayer times for $city, $country ($tz):"
Write-Host "Fajr $fajr | Sunrise $sunrise | Dhuhr $dhuhr | Asr $asr | Maghrib $maghrib | Isha $isha"

# 5) Loop: stream current time every second (local system time)
try {
  while ($true) {
    $now = Get-Date -Format "HH:mm:ss"
    Send-Line ("TIME|{0}" -f $now)

    # At midnight, re-fetch for the new day
    if ((Get-Date).Hour -eq 0 -and (Get-Date).Minute -eq 0 -and (Get-Date).Second -lt 2) {
      try {
        $resp = Invoke-RestMethod -Uri $uri -TimeoutSec 10 -ErrorAction Stop
        $tim  = $resp.data.timings
        $fajr    = HHMM $tim.Fajr
        $sunrise = HHMM $tim.Sunrise
        $dhuhr   = HHMM $tim.Dhuhr
        $asr     = HHMM $tim.Asr
        $maghrib = HHMM $tim.Maghrib
        $isha    = HHMM $tim.Isha
        $line = "PRAYERS|Fajr|$fajr|Sunrise|$sunrise|Dhuhr|$dhuhr|Asr|$asr|Maghrib|$maghrib|Isha|$isha"
        Send-Line $line
      } catch { }
    }

    Start-Sleep -Seconds 1
  }
}
finally {
  if ($sp -and $sp.IsOpen) { $sp.Close() }
}
