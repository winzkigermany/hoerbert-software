function Controller()
{
  console.log("installer controller 1740");
  console.log("OS: " + systemInfo.productType);
  console.log("Kernel: " + systemInfo.kernelType + "/" + systemInfo.kernelVersion);
  console.log("CPU Architecture: " +  systemInfo.currentCpuArchitecture);

  installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
  installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
  installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);  

  installer.gainAdminRights();
}

var Dir = new function () {
    this.toNativeSeparator = function (path) {
        if (systemInfo.productType === "windows")
            return path.replace(/\//g, '\\');
        return path;
    }
};

Controller.prototype.IntroductionPageCallback = function()
{
	console.log("IntroductionPageCallback");

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

}



Controller.prototype.TargetDirectoryPageCallback = function()
{
  console.log("TargetDirectoryPageCallback");

  if( installer.isInstaller() )
  {
  	installer.setValue("RemoveTargetDir",false);	// this makes it possible to re-select an existing target directory
  }
}



Controller.prototype.PerformInstallationPageCallback = function()
{
  	console.log("PerformInstallationPageCallback");

  	// first, we need to remove the old installation if it exists.
	var targetDirectory = installer.value("TargetDir");
	if (installer.fileExists(targetDirectory) && installer.fileExists(targetDirectory + "/maintenancetool.exe")) {
	    console.log("Target directory already exists. Try to uninstall first.");

//	    var returnValue = installer.execute(targetDirectory + "/maintenancetool.exe", "--sr --am -c purge");
//		var parameterString = '/C start "uninstall" /B "'+targetDirectory+'/maintenancetool.exe" --sr --am -c purge';
//		var parameterString = "/C /s start /b '"+targetDirectory+"/maintenancetool.exe' --sr --am -c purge";
//		console.log("Executing: cmd "+parameterString);
//      var returnValue = installer.execute( "cmd", parameterString);
//	    var returnValue = installer.execute( "cmd", ["/C","/s","start","uninstall","/b", "\""+targetDirectory+"/maintenancetool.exe\"", "--sr","--am","-c","purge"]);
//      var returnValue = installer.performOperation( "Execute", ["cmd","/s","/C","start","uninstall","/b",targetDirectory+"/maintenancetool.exe", "--sr","--am","-c","purge"]);
      var returnValue = installer.performOperation( "Execute", ["cmd","/C","start",'"^\"',"/B",Dir.toNativeSeparator(targetDirectory+"/maintenancetool.exe"),"--sr","--am","-c","purge"]);

	    if( returnValue.length>0 )	// maintenanceTool was invoked
	    {
	    	console.log("Uninstaller returned result: "+returnValue[1]);
	    }
	    else
	    {
	    	console.log("maintenancetool.exe was not invoked. Could be a problem.");
	    }

	}
}