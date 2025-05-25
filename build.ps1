# Minecraft Borderless Tool Build Script

param(
    [switch]$Clean,
    [switch]$Rebuild,
    [switch]$Install,
    [switch]$Help
)

# 設定
$SrcDir = "src"
$IncludeDir = "include"
$BuildDir = "build"
$ObjDir = "$BuildDir\obj"
$Target = "$BuildDir\MinecraftBorderless.exe"
$LegacyTarget = "MinecraftBorderless.exe"

# コンパイラ設定
$CXX = "g++"
$CxxFlags = @(
    "-std=c++17",
    "-Wall",
    "-Wextra", 
    "-O2",
    "-mwindows",
    "-I$IncludeDir"
)
$Libs = @("-lcomctl32", "-lshell32", "-lshlwapi")

function Show-Help {
    Write-Host "Minecraft Borderless Tool Build Script"
    Write-Host ""
    Write-Host "Usage:"
    Write-Host "  .\build.ps1                - Build project"
    Write-Host "  .\build.ps1 -Clean         - Clean build files"
    Write-Host "  .\build.ps1 -Rebuild       - Clean rebuild"
    Write-Host "  .\build.ps1 -Install       - Install executable"
    Write-Host "  .\build.ps1 -Help          - Show this help"
}

function Remove-BuildFiles {
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "Build files cleaned up"
    }
}

function Remove-LegacyFiles {
    if (Test-Path $LegacyTarget) {
        Remove-Item $LegacyTarget
        Write-Host "Legacy files cleaned up"
    }
}

function New-BuildDirectories {
    if (!(Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }
    if (!(Test-Path $ObjDir)) {
        New-Item -ItemType Directory -Path $ObjDir | Out-Null
    }
}

function Build-Project {
    Write-Host "Starting build..."
    
    # ディレクトリを作成
    New-BuildDirectories
    
    # ソースファイルを取得
    $SourceFiles = Get-ChildItem -Path $SrcDir -Filter "*.cpp"
    $ObjectFiles = @()
    
    # 各ソースファイルをコンパイル
    foreach ($SourceFile in $SourceFiles) {
        $ObjectFile = "$ObjDir\$($SourceFile.BaseName).o"
        $ObjectFiles += $ObjectFile
        
        Write-Host "Compiling: $($SourceFile.Name)"
        
        $CompileArgs = @($CxxFlags) + @("-c", $SourceFile.FullName, "-o", $ObjectFile)
        
        & $CXX @CompileArgs
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Compile error: $($SourceFile.Name)"
            return $false
        }
    }
    
    # Link
    Write-Host "Linking..."
    $LinkArgs = @($ObjectFiles) + @("-o", $Target) + $Libs + $CxxFlags
    
    & $CXX @LinkArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Link error"
        return $false
    }
    
    Write-Host "Build complete: $Target"
    return $true
}

function Install-Executable {
    if (Test-Path $Target) {
        Copy-Item $Target -Destination "."
        Write-Host "Executable installed"
    } else {
        Write-Error "Executable not found. Please build first."
    }
}

# メイン処理
if ($Help) {
    Show-Help
    exit 0
}

if ($Clean) {
    Remove-BuildFiles
    Remove-LegacyFiles
    exit 0
}

if ($Rebuild) {
    Remove-BuildFiles
}

# ビルド実行
$BuildSuccess = Build-Project

if ($BuildSuccess -and $Install) {
    Install-Executable
}

if ($BuildSuccess) {
    Write-Host "Build completed successfully!"
} else {
    Write-Error "Build failed."
    exit 1
} 