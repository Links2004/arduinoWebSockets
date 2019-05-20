Welcome to Visual Micro shared Arduino library projects

  This file can be deleted after it has been read.

  Normally, for Arduino, libraries have to exist in one of a few specified folder(s). However a Visual Studio shared project library can exist in any location.

  Another benefit of shared library projects is that they support the standard Visual Micro debugging.

About shared Arduino library shared projects

  A Visual Studio shared project library is one that simply contains a Visual Studio .vxitems file instead of the usual .vcxproj file. These are the "Project" files that contain the links to the source code shown in the Solution Explorer.

  An existing ".vcxitems" file can be be added to any Visual Studio solution using "File>Add>Existing Project" or created using "File>New>Project>C++>Misc>Shared Project"

  Visual Micro has created this one for you along with a .vcxitems file. 

  To ensure this library remains compatible with both Arduino and Visual Micro ensure that the library folder name, the .vcxitems file name and a .h name must all match. Also ensure that an Arduino format "library.properties" file exists in this folder. 

  If you would like this library to be organised into a virtual folder with other libraries the create a new "Solution Folder" using the Solution Explorer content menu and drag/move this library folder.

  If you want to move/copy this library to a different location then remove it from this solution, physically move this folder using windows explorer and then use "File>Add>Existing Project" to re-add to this solution. Visual Studio will not auto detect the move so you will need to remove the "Shared Project Reference" you added to any arduino projects and then re-add the Reference.

!!IMPORTANT NOTE!!

  To ensure that Visual Studio intellisense works correctly with a shared project or shared library it is IMPORTANT to add a "Reference" from your standard Arduino project(s) to this library. To this by right clicking the project, click "Add Reference". When the Add Reference window opens click "Shared Projects" and you will see your shared project in the list. Check it and click OK.

  When editing shared project code, you will find that shared projects can not be started directly and are not associated to specific hardware, board or architecure. Instead, intellisense will dynamically adjust for the current "Visual Studio Startup Project".

  More information is here https://www.visualmicro.com/post/2017/01/16/Arduino-Cross-Platform-Library-Development.aspx

