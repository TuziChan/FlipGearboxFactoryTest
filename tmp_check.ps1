$f1 = [System.IO.File]::ReadAllText("f:\Work\FlipGearboxFactoryTest\src\ui\components\AppTable.qml")
$f2 = [System.IO.File]::ReadAllText("f:\Work\FlipGearboxFactoryTest\src\ui\components\MetricCard.qml")
$doubleCR = "`r`r`n"
Write-Host "AppTable doubleCR: $($f1.Contains($doubleCR))"
Write-Host "MetricCard doubleCR: $($f2.Contains($doubleCR))"
