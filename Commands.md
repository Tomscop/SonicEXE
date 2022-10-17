make -f Makefile.tools
make -f Makefile.assets -j8
make
mkpsxiso -y funkin.xml