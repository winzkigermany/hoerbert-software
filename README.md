# hoerbert-software
Hörbert playlist management software. 

Our hörbert-software is a tool to manage and transfer audio data for hörbert (https://www.hoerbert.com) memory cards.  
It mainly helps users to put contents of hörbert playlists into the right order.  
Also during transfer, the files are converted to a format that hörbert plays with the least amount of energy - which results in a veeery long playback time.  

This is a Qt Project which can be built with Qt Creator 4. 
... on a mac using the kit: Desktop Qt 5.1x clang 64 bit  
... on windows using the kit: Desktop Qt 5.1x MSVC 2019 64 bit and 32 bit  
... on linux using the kit: Desktop Qt 5.1x GCC 64 bit  

Packaging is also included in this project through scripts, although it might be not-so-easy-to use,  
because there *are* fixed paths and whatnot inside those scripts. However, they should give you 
an idea of the steps to take.  
... on linux, to create an AppImage, use linux-deploy.sh  
... on windows, to create an installer, use win-deploy-64.bat and win-deploy-32.bat  
... on mac, use mac-deploy.sh to create an app. Notarizing and packing a dmg is not included here.  
(We use SD Notary https://latenightsw.com/sd-notary-notarizing-made-easy/ and create-dmg https://github.com/create-dmg/create-dmg)  

Have fun poking around,  
your hörbert team
