function Component() 
{
  // start installer with -d to see debug output
  console.log("installer32 1644");

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

Component.prototype.createOperations = function() 
{
  console.log("createOperations");
	component.createOperations();

 	if ( installer.value("os") === "win" ) 
  {
  	component.addOperation( "CreateShortcut", "@TargetDir@/hoerbert.exe", "@StartMenuDir@/hoerbert.lnk" );
  }
}
