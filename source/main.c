#include "myGUI.h"
#include "editorCore.h"
#include "explorerCore.h"
#include "fileSystem.h"
#include "controller.h"
#include "blocklistInitor.h"
#include "search.h"

void Main()
{
	ExploreCoreInitCallBacks();
	EditorCoreInitCallbacks();
	ControllerInitCallbacks();
	BlockListInitMethods();

	RegisterSavFilePath("user.sav");
	LoadSavFile();
	BuildFiles();
	SearchBuild();

	GUI_Start();
}
