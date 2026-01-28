#define MyAppVersion "0.1.0"
[Setup]
AppName=CentroidRolloffVisualizer
AppVersion={#MyAppVersion}
DefaultDirName="{commoncf}\VST3"
DefaultGroupName=CentroidRolloffVisualizer
OutputBaseFilename=CentroidRolloffVisualizer
UsePreviousAppDir=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
CreateUninstallRegKey=no
Uninstallable=no
DisableDirPage=no
DisableProgramGroupPage=yes
DisableReadyMemo=yes
[Files]
Source: "{#SourcePath}build\CentroidRolloffVisualizer_artefacts\Release\VST3\CentroidRolloffVisualizer.vst3\*"; DestDir: "{app}\CentroidRolloffVisualizer.vst3"; Flags: recursesubdirs restartreplace ignoreversion