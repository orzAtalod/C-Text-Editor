#include "myGUI.h"
#include "editorCore.h"
#include "fileSystem.h"
#include "controller.h"

void Main()
{
	EditorCoreInitCallbacks();
	ControllerInitCallbacks();
	BlockListInitMethods();

	RegisterSavFilePath("user.sav");
	LoadSavFile();
	BuildFiles();
	fclose(f);

	GUI_Start();
}