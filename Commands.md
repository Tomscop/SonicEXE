make -f Makefile.tools
make -f Makefile.assets -j6
make
mkpsxiso -y funkin.xml