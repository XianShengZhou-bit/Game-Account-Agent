$basePath = $PSScriptRoot
$games = Get-Content -Path "$basePath\games.txt" -Encoding UTF8
$servers = Get-Content -Path "$basePath\servers.txt" -Encoding UTF8
$rareItems = Get-Content -Path "$basePath\rares.txt" -Encoding UTF8
$titles = Get-Content -Path "$basePath\titles.txt" -Encoding UTF8
$rng = New-Object System.Random
$sql = 'INSERT INTO accounts (game_name, server_area, title, price, account_level, hero_count, skin_count, rare_items, status, version, created_at) VALUES'
$values = @()
$startDate = Get-Date "2000-01-01"
$endDate = Get-Date
$timeSpan = $endDate - $startDate

for ($i = 1; $i -le 1000; $i++) {
    $game = $games[$rng.Next($games.Length)]
    $server = $servers[$rng.Next($servers.Length)]
    $title = $titles[$rng.Next($titles.Length)]
    $price = $rng.Next(10000, 100000000)
    $level = $rng.Next(1, 501)
    $heroCount = $rng.Next(0, 501)
    $skinCount = $rng.Next(0, 1001)
    $rareCount = $rng.Next(0, 11)
    $selectedRares = @()
    for ($j = 0; $j -lt $rareCount; $j++) {
        $selectedRares += $rareItems[$rng.Next($rareItems.Length)]
    }
    if ($selectedRares.Length -eq 0) {
        $rareJson = '[]'
    } else {
        $rareJson = '["' + ($selectedRares -join '","') + '"]'
    }
    $status = 0
    $version = 0
    $randomDays = $rng.Next(0, $timeSpan.Days)
    $createdAt = $startDate.AddDays($randomDays).ToString('yyyy-MM-dd HH:mm:ss')
    $values += "('$game', '$server', '$title', $price, $level, $heroCount, $skinCount, '$rareJson', $status, $version, '$createdAt')"
}
$sql += "`n" + ($values -join ",`n") + ';'
$sql | Out-File -FilePath "$basePath\generated_data.sql" -Encoding UTF8