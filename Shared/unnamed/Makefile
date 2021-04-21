# Run

# Add #include <unnamed/FILE_NAME>
# Compile with the flag -lunnamed

# INSTALACION DE LIBRERIAS COMPARTIDAS

# sudo make install
install: 
	# Install the Shared Libraries
	
	# Compiling all .c
	gcc -Wall -fPIC -c *.c
	# Compiling all .o
	gcc -shared -Wl,-soname,libunnamed.so -o libunnamed.so *.o
	# Moving the lib to the global link route
	cp -u libunnamed.so /usr/lib
	cp -R -u ../unnamed /usr/include
	# Removing obj files..
	rm libunnamed.so *.o
	# Done.

# sudo make uninstall
uninstall:
	# Removing files..
	rm -f /usr/lib/libunnamed.so
	rm -rf /usr/include/unnamed
	# Done.

# sudo make update
update: uninstall install
