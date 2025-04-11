# Set the source and output directories
$SRC_DIR = "C:\Users\masit\Documents\GitHub\OWN_CLOX_IMPLEMENTATION\src"
$OUT_DIR = "C:\Users\masit\Documents\GitHub\OWN_CLOX_IMPLEMENTATION\output"

# Ensure output directory exists
if (-not (Test-Path $OUT_DIR)) {
    New-Item -ItemType Directory -Force -Path $OUT_DIR | Out-Null
} else {
    # Clean output directory
    Get-ChildItem -Path $OUT_DIR -Recurse -File | Remove-Item -Force
}

# Get all C source files from the source directory
$sourceFiles = Get-ChildItem -Path $SRC_DIR -Filter *.c

# Compile all source files into object files (.obj for Windows/tcc)
$objectFiles = @()
foreach ($sourceFile in $sourceFiles) {
    $objectFile = Join-Path $OUT_DIR ($sourceFile.BaseName + ".obj")
    & tcc -DDEBUG_TRACE_EXECUTION -c "`"$($sourceFile.FullName)`"" -o "`"$objectFile`""
    $objectFiles += "`"$objectFile`""
}

# Link all object files into a single executable
$outputExe = Join-Path $OUT_DIR "clox.exe"
& tcc $objectFiles -o "`"$outputExe`""

# Run the compiled executable
& "`"$outputExe`""
