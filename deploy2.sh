echo -e "\n\nInstalling Commons libraries...\n\n"

COMMONS="so-commons-library"

rm -rf $COMMONS
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD

echo -e "\n\Installing Unnamed libraries..."

UNNAMED="Shared/unnamed/"

cd $UNNAMED
make install

echo -e "\n Installing Done!\n"
