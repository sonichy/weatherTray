s="[Desktop Entry]\nName=天气预报\nComment=托盘天气预报\nExec=`pwd`/weatherTray\nIcon=`pwd`/icon.png\nPath=`pwd`\nTerminal=false\nType=Application\nCategories=Network;"
echo -e $s > weatherTray.desktop
cp `pwd`/weatherTray.desktop ~/.config/autostart/weatherTray.desktop