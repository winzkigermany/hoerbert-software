// constructor
function Component() {
     var programFiles = installer.environmentVariable("ProgramFiles");
     if( programFiles!="" ){
         installer.setValue("TargetDir", programFiles + "/hoerbert");
     }
}


Component.prototype.createOperations = function() {
	component.createOperations();

 	if ( installer.value("os") === "win" ) {
  		component.addOperation(
  			"CreateShortcut",
  			"@TargetDir@/hoerbert.exe",
  			"@StartMenuDir@/hoerbert.lnk"
  		);
  	}
 }
