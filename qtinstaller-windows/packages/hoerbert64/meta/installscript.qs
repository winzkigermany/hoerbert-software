function Component() 
{
  // start installer with -d to see debug output
  console.log("installer64");
  console.log("OS: " + systemInfo.productType);
  console.log("Kernel: " + systemInfo.kernelType + "/" + systemInfo.kernelVersion);
  console.log("CPU Architecture: " +  systemInfo.currentCpuArchitecture);

  installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
  installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
  installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);  

  installer.gainAdminRights();

  // keep the 32 bit installer from installing apps to the (x86) program files directory
  if ( installer.value("os") === "win" ) {
    var programFiles = installer.environmentVariable("ProgramW6432");   // should be "Program Files" on 64 bit platforms
    if( programFiles!="" )
    {
      console.log("Got Program Files folder from %ProgramW6432%: " + programFiles);
    }
    else
    {
      programFiles = installer.environmentVariable("ProgramFiles");   // should be "Program Files" on 32 bit platforms
      console.log("Got Program Files folder from %ProgramFiles%: " + programFiles);
    }
  }
  if( programFiles!="" ){
    var targetDir = programFiles + "/hoerbert";
    console.log("Setting TargetDir: " + targetDir);
    installer.setValue("TargetDir", targetDir);
  }

  //
  // Hide/select packages based on architecture
  //
  // Marking a component as "Virtual" will hide it in the UI.
  // Marking a component with "Default" will check it.
  //
  installer.componentByName("hoerbert32").setValue("Virtual", "true");
  installer.componentByName("hoerbert64").setValue("Virtual", "true");

  if ( systemInfo.currentCpuArchitecture === "x86_64") 
  {
      console.log("Hiding 32 bit version");
      installer.componentByName("hoerbert64").setValue("Virtual", "false");
      installer.componentByName("hoerbert64").setValue("Default", "true");
  }
  else      //if ( systemInfo.currentCpuArchitecture === "i386") 
  {
      console.log("Hiding 64 bit version");
      installer.componentByName("hoerbert32").setValue("Virtual", "false");
      installer.componentByName("hoerbert32").setValue("Default", "true");
  } 
}


Component.prototype.TargetDirectoryPageCallback = function()
{
  if( installer.isInstaller() )
  {

  }

  console.log("TargetDirectoryPageCallback");

  var targetDirectory = installer.value("TargetDir");
  if( installer.fileExists(targetDirectory) )
  {
    console.log("Target directory already exists. Try to uninstall first.");

    //setUninstaller();
    runUninstaller();
    //TODO - execute: maintenancetool.exe --sr --am -c purge
  }
}

Component.prototype.createOperations = function() 
{
  console.log("createOperations");
	component.createOperations();

 	if ( installer.value("os") === "win" ) 
  {
  	component.addOperation( "CreateShortcut", "@TargetDir@/hoerbert.exe", "@StartMenuDir@/hoerbert.lnk" );
  }
}
