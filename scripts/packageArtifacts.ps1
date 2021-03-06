$ErrorActionPreference = "Stop"

$DIST = "artifacts/dist/win64/"

New-Item -Path "$DIST/module-sources" -ItemType "directory"
New-Item -Path "$DIST/native-module-sources" -ItemType "directory"
foreach($element in "pharo-vm",
    "docs", "samples", "tests",
    "README.md", "LICENSE",
    "Pharo*.sources", "sysmel.image", "sysmel.changes",
    "sysmelc.bat", "sysmelc-ui.bat"
) {
    Copy-Item -Recurse -Path "$element" -Destination "$DIST"
}

Copy-Item -Recurse -Path "module-sources/*" -Destination "$DIST/module-sources"
Copy-Item -Recurse -Path "native-module-sources/*" -Destination "$DIST/native-module-sources"
Copy-Item -Recurse -Path "pharo-local/iceberg/**/abstract-gpu/bindings/sysmel/module-sources/*" -Destination "$DIST/module-sources"
Copy-Item -Recurse -Path "pharo-local/iceberg/**/abstract-gpu/bindings/sysmel/native-module-sources/*" -Destination "$DIST/native-module-sources"
Copy-Item -Recurse -Path "pharo-local/phanapi/libs/abstract-gpu/*" -Destination "$DIST/native-module-sources/abstract-gpu"
